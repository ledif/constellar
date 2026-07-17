#pragma once

#include <QObject>

// Exercises DiscordIpcClient's wire protocol against a fake Discord IPC
// server (QLocalServer standing in for the real desktop client), per
// ADR-010. Covers the handshake and a SET_ACTIVITY round trip; reconnect
// backoff isn't timing-tested here to keep the suite fast.
class DiscordIpcClientTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void init();
    void cleanup();

    void sendsHandshakeOnConnect();
    void becomesReadyOnDispatchReady();
    void sendsSetActivityAfterReady();
    void clearActivitySendsNullActivity();

  private:
    class QTemporaryDir* m_dir = nullptr;
};
