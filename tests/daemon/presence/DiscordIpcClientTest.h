#pragma once

#include <QObject>

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
    void throttleCoalescesRapidUpdates();
    void reconnectsAfterServerDrop();
    void respondsToPingWithPong();
    void closeOpcodeDisconnects();
    void reassemblesFrameSplitAcrossReads();

  private:
    class QTemporaryDir* m_dir = nullptr;
};
