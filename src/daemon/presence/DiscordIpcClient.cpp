#include "DiscordIpcClient.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QUuid>

using namespace Qt::StringLiterals;

namespace
{

constexpr qint32 kOpHandshake = 0;
constexpr qint32 kOpFrame = 1;
constexpr qint32 kOpClose = 2;
constexpr qint32 kOpPing = 3;
constexpr qint32 kOpPong = 4;

// Discord recommends 1 update per 15s
// https://github.com/discord/discord-api-docs/issues/668
constexpr int kThrottleIntervalMs = 15'000;

constexpr int kReconnectIntervalMs = 5'000;

}  // namespace

DiscordIpcClient::DiscordIpcClient(QString appId, QObject* parent)
    : QObject(parent), m_appId(std::move(appId))
{
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &DiscordIpcClient::attemptConnect);

    m_throttleTimer.setSingleShot(true);
    connect(&m_throttleTimer, &QTimer::timeout, this, &DiscordIpcClient::flushThrottle);

    connect(&m_socket, &QLocalSocket::connected, this, &DiscordIpcClient::onConnected);
    connect(&m_socket, &QLocalSocket::readyRead, this, &DiscordIpcClient::onReadyRead);
    connect(&m_socket, &QLocalSocket::disconnected, this, &DiscordIpcClient::onDisconnected);
    connect(&m_socket, &QLocalSocket::errorOccurred, this, &DiscordIpcClient::onSocketError);
}

void DiscordIpcClient::start()
{
    if (m_appId.isEmpty())
    {
        qDebug() << "DiscordIpcClient: no app ID configured, presence disabled";
        return;
    }

    attemptConnect();
}

bool DiscordIpcClient::isReady() const
{
    return m_ready;
}

void DiscordIpcClient::setActivity(QJsonObject const& activity)
{
    m_pendingActivity = QJsonValue(activity);
    sendPending();
}

void DiscordIpcClient::clearActivity()
{
    m_pendingActivity = QJsonValue(QJsonValue::Null);
    sendPending();
}

void DiscordIpcClient::sendPending()
{
    // nothing to send
    if (!m_ready || !m_pendingActivity.has_value())
        return;

    // a send is already in-flight for this window
    if (m_throttleTimer.isActive())
        return;

    // we gucci

    QJsonObject args;
    args["pid"] = QCoreApplication::applicationPid();
    args["activity"] = *m_pendingActivity;

    QJsonObject command;
    command["cmd"] = u"SET_ACTIVITY"_s;
    command["args"] = args;
    command["nonce"] = QUuid::createUuid().toString(QUuid::WithoutBraces);

    sendFrame(kOpFrame, command);
    m_pendingActivity.reset();
    m_throttleTimer.start(kThrottleIntervalMs);
}

void DiscordIpcClient::flushThrottle()
{
    sendPending();
}

void DiscordIpcClient::attemptConnect()
{
    if (m_appId.isEmpty() || m_socket.state() != QLocalSocket::UnconnectedState)
        return;

    m_socket.connectToServer(socketPath(m_socketIndex));
}

void DiscordIpcClient::onConnected()
{
    QJsonObject handshake;
    handshake["v"] = 1;
    handshake["client_id"] = m_appId;
    sendFrame(kOpHandshake, handshake);
}

void DiscordIpcClient::onReadyRead()
{
    m_readBuffer.append(m_socket.readAll());

    while (m_readBuffer.size() >= 8)
    {
        QDataStream header(m_readBuffer);
        header.setByteOrder(QDataStream::LittleEndian);
        qint32 opcode = 0;
        qint32 length = 0;
        header >> opcode >> length;

        if (length < 0 || m_readBuffer.size() < 8 + length)
            break;  // wait for the rest of the frame

        QByteArray const json = m_readBuffer.mid(8, length);
        m_readBuffer.remove(0, 8 + length);

        QJsonDocument const doc = QJsonDocument::fromJson(json);
        handleFrame(opcode, doc.object());
    }
}

void DiscordIpcClient::handleFrame(qint32 opcode, QJsonObject const& payload)
{
    switch (opcode)
    {
        case kOpFrame:
            if (payload.value("cmd").toString() == u"DISPATCH"_s &&
                payload.value("evt").toString() == u"READY"_s)
            {
                m_ready = true;
                Q_EMIT ready();
                sendPending();
            }
            break;
        case kOpPing:
            sendFrame(kOpPong, payload);
            break;
        case kOpClose:
            m_socket.disconnectFromServer();
            break;
        default:
            break;
    }
}

void DiscordIpcClient::onDisconnected()
{
    m_ready = false;
    m_readBuffer.clear();
    Q_EMIT lost();
    scheduleReconnect();
}

void DiscordIpcClient::onSocketError()
{
    m_ready = false;
    m_readBuffer.clear();
    // Discord not running (or not on this socket index) is the normal
    // case -- cycle through discord-ipc-0..9 rather than giving up.
    m_socketIndex = (m_socketIndex + 1) % 10;
    scheduleReconnect();
}

void DiscordIpcClient::scheduleReconnect()
{
    if (m_socket.state() != QLocalSocket::UnconnectedState)
        m_socket.abort();
    m_reconnectTimer.start(kReconnectIntervalMs);
}

void DiscordIpcClient::sendFrame(qint32 opcode, QJsonObject const& payload)
{
    QByteArray const json = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << opcode << static_cast<qint32>(json.size());
    frame.append(json);

    m_socket.write(frame);
}

QString DiscordIpcClient::socketPath(int index)
{
    QString base = qEnvironmentVariable("XDG_RUNTIME_DIR");

    if (base.isEmpty())
        base = qEnvironmentVariable("TMPDIR");  // arbiter save you

    return base + u"/discord-ipc-%1"_s.arg(index);
}
