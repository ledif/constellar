#include "DiscordIpcClientTest.h"

#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "DiscordIpcClient.h"

namespace {

QByteArray encodeFrame(qint32 opcode, const QJsonObject &payload) {
    const QByteArray json = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << opcode << static_cast<qint32>(json.size());
    frame.append(json);
    return frame;
}

// Reads exactly one frame off a socket that's already known to have one
// buffered (caller QTRY_VERIFYs bytesAvailable() first).
bool decodeFrame(QLocalSocket &socket, qint32 &opcode, QJsonObject &payload) {
    if (socket.bytesAvailable() < 8) {
        return false;
    }
    const QByteArray header = socket.peek(8);
    QDataStream headerStream(header);
    headerStream.setByteOrder(QDataStream::LittleEndian);
    qint32 length = 0;
    headerStream >> opcode >> length;

    if (socket.bytesAvailable() < 8 + length) {
        return false;
    }
    socket.read(8);
    const QByteArray json = socket.read(length);
    payload = QJsonDocument::fromJson(json).object();
    return true;
}

}  // namespace

void DiscordIpcClientTest::init() {
    m_dir = new QTemporaryDir();
    QVERIFY(m_dir->isValid());
    qputenv("XDG_RUNTIME_DIR", m_dir->path().toUtf8());
}

void DiscordIpcClientTest::cleanup() {
    delete m_dir;
    m_dir = nullptr;
}

void DiscordIpcClientTest::sendsHandshakeOnConnect() {
    const QString socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("999999999999999999"));
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket *peer = server.nextPendingConnection();
    QVERIFY(peer != nullptr);

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));

    QCOMPARE(opcode, 0);  // HANDSHAKE
    QCOMPARE(payload.value("v").toInt(), 1);
    QCOMPARE(payload.value("client_id").toString(), QStringLiteral("999999999999999999"));
}

void DiscordIpcClientTest::becomesReadyOnDispatchReady() {
    const QString socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("1"));
    QSignalSpy readySpy(&client, &DiscordIpcClient::ready);
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket *peer = server.nextPendingConnection();
    QTRY_VERIFY(peer->bytesAvailable() >= 8);  // drain the handshake frame
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));

    QJsonObject data;
    QJsonObject readyEvt;
    readyEvt["cmd"] = QStringLiteral("DISPATCH");
    readyEvt["evt"] = QStringLiteral("READY");
    readyEvt["data"] = data;
    peer->write(encodeFrame(1, readyEvt));
    peer->flush();

    QTRY_COMPARE(readySpy.count(), 1);
    QVERIFY(client.isReady());
}

void DiscordIpcClientTest::sendsSetActivityAfterReady() {
    const QString socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("1"));
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket *peer = server.nextPendingConnection();
    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));  // handshake

    QJsonObject readyEvt;
    readyEvt["cmd"] = QStringLiteral("DISPATCH");
    readyEvt["evt"] = QStringLiteral("READY");
    peer->write(encodeFrame(1, readyEvt));
    peer->flush();
    QTRY_VERIFY(client.isReady());

    QJsonObject activity;
    activity["details"] = QStringLiteral("Mythic Ulgrax the Devourer");
    client.setActivity(activity);

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));

    QCOMPARE(opcode, 1);  // FRAME
    QCOMPARE(payload.value("cmd").toString(), QStringLiteral("SET_ACTIVITY"));
    const QJsonObject args = payload.value("args").toObject();
    QCOMPARE(args.value("activity").toObject().value("details").toString(),
             QStringLiteral("Mythic Ulgrax the Devourer"));
}

void DiscordIpcClientTest::clearActivitySendsNullActivity() {
    const QString socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("1"));
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket *peer = server.nextPendingConnection();
    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));  // handshake

    QJsonObject readyEvt;
    readyEvt["cmd"] = QStringLiteral("DISPATCH");
    readyEvt["evt"] = QStringLiteral("READY");
    peer->write(encodeFrame(1, readyEvt));
    peer->flush();
    QTRY_VERIFY(client.isReady());

    client.clearActivity();

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));

    const QJsonObject args = payload.value("args").toObject();
    QVERIFY(args.value("activity").isNull());
}

QTEST_MAIN(DiscordIpcClientTest)
