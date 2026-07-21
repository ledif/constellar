#include "LogWatcherTest.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QVector>

#include "LogWatcher.h"

namespace
{

class ReceivedLines
{
  public:
    explicit ReceivedLines(LogWatcher& watcher)
    {
        QObject::connect(
            &watcher, &LogWatcher::lineReceived,
            [this](LogLine const& line) { m_lines.append(line.raw()); }
        );
    }

    QVector<QString> const& lines() const
    {
        return m_lines;
    }

  private:
    QVector<QString> m_lines;
};

QString timestampPrefix()
{
    return QStringLiteral("7/27/2024 21:39:13.0951  ");
}

}  // namespace

void LogWatcherTest::tailsNewWrites()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    LogWatcher watcher(dir.path().toStdString());
    ReceivedLines received(watcher);
    QVERIFY(watcher.start());

    QFile file(dir.filePath(QStringLiteral("WoWCombatLog.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write((timestampPrefix() + QStringLiteral("COMBAT_LOG_VERSION,20\n")).toUtf8());
    file.write((timestampPrefix() + QStringLiteral("ZONE_CHANGE,1,\"Foo\",1\n")).toUtf8());
    file.close();

    QTRY_COMPARE_WITH_TIMEOUT(received.lines().size(), 2, 2000);
    QVERIFY(received.lines().at(0).contains(QStringLiteral("COMBAT_LOG_VERSION")));
    QVERIFY(received.lines().at(1).contains(QStringLiteral("ZONE_CHANGE")));
}

void LogWatcherTest::handlesPartialLineAtEof()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    LogWatcher watcher(dir.path().toStdString());
    ReceivedLines received(watcher);
    QVERIFY(watcher.start());

    QFile file(dir.filePath(QStringLiteral("WoWCombatLog.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write((timestampPrefix() + QStringLiteral("ENCOUNTER_START,1")).toUtf8());
    file.flush();

    // No trailing newline yet: nothing should be emitted.
    QTest::qWait(300);
    QCOMPARE(received.lines().size(), 0);

    file.write(QByteArrayLiteral(",2,3,4,5\n"));
    file.close();

    QTRY_COMPARE_WITH_TIMEOUT(received.lines().size(), 1, 2000);
    QCOMPARE(
        received.lines().at(0), timestampPrefix() + QStringLiteral("ENCOUNTER_START,1,2,3,4,5")
    );
}

void LogWatcherTest::resetsOffsetOnFileRecreation()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    LogWatcher watcher(dir.path().toStdString());
    ReceivedLines received(watcher);
    QVERIFY(watcher.start());

    QString const path = dir.filePath(QStringLiteral("WoWCombatLog.txt"));

    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write((timestampPrefix() + QStringLiteral("ZONE_CHANGE,1,\"A\",1\n")).toUtf8());
    }
    QTRY_COMPARE_WITH_TIMEOUT(received.lines().size(), 1, 2000);

    QFile::remove(path);
    QTest::qWait(100);

    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write((timestampPrefix() + QStringLiteral("ZONE_CHANGE,2,\"B\",2\n")).toUtf8());
    }
    QTRY_COMPARE_WITH_TIMEOUT(received.lines().size(), 2, 2000);
    QVERIFY(received.lines().at(1).contains(QStringLiteral("\"B\"")));
}

void LogWatcherTest::ignoresNonCombatLogFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    LogWatcher watcher(dir.path().toStdString());
    ReceivedLines received(watcher);
    QVERIFY(watcher.start());

    QFile file(dir.filePath(QStringLiteral("notes.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("not a combat log\n");
    file.close();

    QTest::qWait(300);
    QCOMPARE(received.lines().size(), 0);
}

void LogWatcherTest::emitsIdleTimeoutAfterInactivity()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    LogWatcher watcher(dir.path().toStdString(), /*idleTimeoutMs=*/100);
    QSignalSpy idleSpy(&watcher, &LogWatcher::idleTimeout);
    QVERIFY(watcher.start());

    QFile file(dir.filePath(QStringLiteral("WoWCombatLog.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write((timestampPrefix() + QStringLiteral("COMBAT_LOG_VERSION,20\n")).toUtf8());
    file.close();

    QVERIFY(idleSpy.wait(2000));
}

void LogWatcherTest::tailsMultipleCombatLogFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    LogWatcher watcher(dir.path().toStdString());
    ReceivedLines received(watcher);
    QVERIFY(watcher.start());

    QFile file1(dir.filePath(QStringLiteral("WoWCombatLog.txt")));
    QVERIFY(file1.open(QIODevice::WriteOnly));
    file1.write((timestampPrefix() + QStringLiteral("ZONE_CHANGE,1,\"A\",1\n")).toUtf8());
    file1.close();

    QFile file2(dir.filePath(QStringLiteral("WoWCombatLog-2.txt")));
    QVERIFY(file2.open(QIODevice::WriteOnly));
    file2.write((timestampPrefix() + QStringLiteral("ZONE_CHANGE,2,\"B\",2\n")).toUtf8());
    file2.close();

    QTRY_COMPARE_WITH_TIMEOUT(received.lines().size(), 2, 2000);
    QVERIFY(received.lines().at(0).contains(QStringLiteral("\"A\"")));
    QVERIFY(received.lines().at(1).contains(QStringLiteral("\"B\"")));
}

QTEST_MAIN(LogWatcherTest)
