#include "DiscordIpcClientTest.h"

#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "DiscordFrame.h"
#include "DiscordIpcClient.h"

void DiscordIpcClientTest::init()
{
    m_dir = new QTemporaryDir();
    QVERIFY(m_dir->isValid());
    qputenv("XDG_RUNTIME_DIR", m_dir->path().toUtf8());
}

void DiscordIpcClientTest::cleanup()
{
    delete m_dir;
    m_dir = nullptr;
}

void DiscordIpcClientTest::sendsHandshakeOnConnect()
{
    QString const socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("999999999999999999"));
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
    QVERIFY(peer != nullptr);

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));

    QCOMPARE(opcode, 0);  // HANDSHAKE
    QCOMPARE(payload.value("v").toInt(), 1);
    QCOMPARE(payload.value("client_id").toString(), QStringLiteral("999999999999999999"));
}

void DiscordIpcClientTest::becomesReadyOnDispatchReady()
{
    QString const socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("1"));
    QSignalSpy readySpy(&client, &DiscordIpcClient::ready);
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
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

void DiscordIpcClientTest::sendsSetActivityAfterReady()
{
    QString const socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("1"));
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
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
    QJsonObject const args = payload.value("args").toObject();
    QCOMPARE(
        args.value("activity").toObject().value("details").toString(),
        QStringLiteral("Mythic Ulgrax the Devourer")
    );
}

void DiscordIpcClientTest::clearActivitySendsNullActivity()
{
    QString const socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("1"));
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
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

    QJsonObject const args = payload.value("args").toObject();
    QVERIFY(args.value("activity").isNull());
}

void DiscordIpcClientTest::throttleCoalescesRapidUpdates()
{
    QString const socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient::Config config;
    config.throttleIntervalMs = 50;
    DiscordIpcClient client(QStringLiteral("1"), config);
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
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

    QJsonObject activityA;
    activityA["details"] = QStringLiteral("A");
    client.setActivity(activityA);

    // The first post-READY update sends immediately and starts the window.
    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));
    QCOMPARE(
        payload.value("args").toObject().value("activity").toObject().value("details").toString(),
        QStringLiteral("A")
    );

    QJsonObject activityB;
    activityB["details"] = QStringLiteral("B");
    client.setActivity(activityB);

    QJsonObject activityC;
    activityC["details"] = QStringLiteral("C");
    client.setActivity(activityC);

    // Still inside the throttle window: nothing new yet.
    QVERIFY(peer->bytesAvailable() < 8);

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));
    QCOMPARE(
        payload.value("args").toObject().value("activity").toObject().value("details").toString(),
        QStringLiteral("C")
    );

    // B was coalesced away: no further frames waiting.
    QVERIFY(!peer->waitForReadyRead(100));
}

void DiscordIpcClientTest::reconnectsAfterServerDrop()
{
    QString const socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient::Config config;
    config.reconnectIntervalMs = 150;
    DiscordIpcClient client(QStringLiteral("1"), config);
    QSignalSpy lostSpy(&client, &DiscordIpcClient::lost);
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));  // handshake

    peer->disconnectFromServer();

    QTRY_COMPARE(lostSpy.count(), 1);

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer2 = server.nextPendingConnection();
    QTRY_VERIFY(peer2->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer2, opcode, payload));
    QCOMPARE(opcode, 0);  // HANDSHAKE
    QCOMPARE(payload.value("client_id").toString(), QStringLiteral("1"));
}

void DiscordIpcClientTest::respondsToPingWithPong()
{
    QString const socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("1"));
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));  // handshake

    QJsonObject pingPayload;
    pingPayload["nonce"] = QStringLiteral("ping-nonce");
    peer->write(encodeFrame(3, pingPayload));  // PING
    peer->flush();

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));
    QCOMPARE(opcode, 4);  // PONG
    QCOMPARE(payload.value("nonce").toString(), QStringLiteral("ping-nonce"));
}

void DiscordIpcClientTest::closeOpcodeDisconnects()
{
    QString const socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("1"));
    QSignalSpy lostSpy(&client, &DiscordIpcClient::lost);
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));  // handshake

    peer->write(encodeFrame(2, QJsonObject{}));  // CLOSE
    peer->flush();

    QTRY_COMPARE(lostSpy.count(), 1);
    QVERIFY(!client.isReady());
}

void DiscordIpcClientTest::reassemblesFrameSplitAcrossReads()
{
    QString const socketPath = m_dir->path() + QStringLiteral("/discord-ipc-0");
    QLocalServer::removeServer(socketPath);
    QLocalServer server;
    QVERIFY(server.listen(socketPath));

    DiscordIpcClient client(QStringLiteral("1"));
    QSignalSpy readySpy(&client, &DiscordIpcClient::ready);
    client.start();

    QTRY_VERIFY(server.hasPendingConnections());
    QLocalSocket* peer = server.nextPendingConnection();
    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    qint32 opcode = -1;
    QJsonObject payload;
    QVERIFY(decodeFrame(*peer, opcode, payload));  // handshake

    QJsonObject readyEvt;
    readyEvt["cmd"] = QStringLiteral("DISPATCH");
    readyEvt["evt"] = QStringLiteral("READY");
    QByteArray const frame = encodeFrame(1, readyEvt);

    // Split the frame across two writes: partial header, then the remainder.
    peer->write(frame.left(4));
    peer->flush();
    QTest::qWait(20);
    peer->write(frame.mid(4));
    peer->flush();

    QTRY_COMPARE(readySpy.count(), 1);
    QVERIFY(client.isReady());

    // Two frames arriving in a single payload should both be handled.
    QJsonObject pingA;
    pingA["nonce"] = QStringLiteral("a");
    QJsonObject pingB;
    pingB["nonce"] = QStringLiteral("b");
    peer->write(encodeFrame(3, pingA) + encodeFrame(3, pingB));
    peer->flush();

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));
    QCOMPARE(payload.value("nonce").toString(), QStringLiteral("a"));

    QTRY_VERIFY(peer->bytesAvailable() >= 8);
    QVERIFY(decodeFrame(*peer, opcode, payload));
    QCOMPARE(payload.value("nonce").toString(), QStringLiteral("b"));
}

QTEST_MAIN(DiscordIpcClientTest)
