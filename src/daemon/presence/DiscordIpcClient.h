#pragma once

#include <optional>

#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QTimer>

// Talks to the local Discord desktop client over a socket at
//   $XDG_RUNTIME_DIR/discord-ipc-{0..9})
class DiscordIpcClient : public QObject
{
    Q_OBJECT

  public:
    struct Config
    {
        int throttleIntervalMs = 15'000;
        int reconnectIntervalMs = 5'000;
        int connectTimeoutMs = 200;
    };

    explicit DiscordIpcClient(QString appId, QObject* parent = nullptr);
    DiscordIpcClient(QString appId, Config config, QObject* parent = nullptr);
    ~DiscordIpcClient() override;

    void start();

    void setActivity(QJsonObject const& activity);
    void clearActivity();

    bool isReady() const;

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
    Config m_config;
    QLocalSocket m_socket;
    QByteArray m_readBuffer;
    bool m_ready = false;
    QTimer m_reconnectTimer;
    QTimer m_throttleTimer;
    std::optional<QJsonValue> m_pendingActivity;
};
