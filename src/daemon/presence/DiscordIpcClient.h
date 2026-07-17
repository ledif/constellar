#pragma once

#include <optional>

#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QTimer>

// Talks to the local Discord desktop client over its Unix IPC socket
// ($XDG_RUNTIME_DIR/discord-ipc-{0..9}), per ADR-010. App-ID-only, no bot/
// OAuth. Discord being absent is the normal case: connection failures are
// silent (qDebug, not qWarning) and just retry on a timer -- presence must
// never affect daemon health (RFC-002).
class DiscordIpcClient : public QObject
{
    Q_OBJECT

  public:
    explicit DiscordIpcClient(QString appId, QObject* parent = nullptr);

    // Starts the connect/handshake/retry loop. No-op if appId is empty.
    void start();

    // Queues a Rich Presence activity payload. Coalesced and throttled to
    // stay under Discord's ~5 updates / 20s SET_ACTIVITY rate limit
    // (ADR-010); the latest call wins if several land inside one window.
    // Silently dropped if not yet connected -- applied once ready() fires.
    void setActivity(QJsonObject const& activity);

    // Clears the activity (e.g. WoW closed). Same coalescing as setActivity.
    void clearActivity();

    bool isReady() const
    {
        return m_ready;
    }

  Q_SIGNALS:
    void ready();
    void lost();

  private Q_SLOTS:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onSocketError();
    void attemptConnect();
    void flushThrottle();

  private:
    void handleFrame(qint32 opcode, QJsonObject const& payload);
    void sendFrame(qint32 opcode, QJsonObject const& payload);
    void sendPending();
    void scheduleReconnect();
    static QString socketPath(int index);

    QString m_appId;
    QLocalSocket m_socket;
    QByteArray m_readBuffer;
    bool m_ready = false;
    int m_socketIndex = 0;
    QTimer m_reconnectTimer;
    QTimer m_throttleTimer;
    std::optional<QJsonValue> m_pendingActivity;
};
