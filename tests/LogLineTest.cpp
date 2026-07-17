#include "LogLineTest.h"

#include <QFile>
#include <QTest>
#include <QTextStream>
#include <QVariantList>

#include "LogLine.h"

namespace
{

QStringList loadFixtureLines()
{
    QFile file(QStringLiteral(LOGFIXTURES_DIR "/sample.txt"));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QStringList lines;
    QTextStream stream(&file);
    while (!stream.atEnd())
    {
        QString const line = stream.readLine();
        if (!line.isEmpty())
            lines.append(line);
    }
    return lines;
}

LogLine findByType(QStringList const& lines, QString const& type)
{
    for (QString const& raw : lines)
    {
        LogLine line(raw);
        if (line.isValid() && line.type() == type)
            return line;
    }
    return LogLine(QString());
}

}  // namespace

void LogLineTest::basicFields()
{
    QStringList const lines = loadFixtureLines();
    QVERIFY(!lines.isEmpty());

    LogLine const encounterStart = findByType(lines, QStringLiteral("ENCOUNTER_START"));
    QVERIFY(encounterStart.isValid());
    QCOMPARE(encounterStart.argCount(), 6);
    QCOMPARE(encounterStart.argString(1), QStringLiteral("3182"));
    QCOMPARE(encounterStart.argString(2), QStringLiteral("Belo'ren, Child of Al'ar"));
    QCOMPARE(encounterStart.argString(3), QStringLiteral("14"));
    QCOMPARE(encounterStart.argString(4), QStringLiteral("11"));
    QCOMPARE(encounterStart.argString(5), QStringLiteral("2913"));
}

void LogLineTest::type()
{
    LogLine line(QStringLiteral("7/27/2024 21:39:13.0951  ENCOUNTER_END,2820,\"Fyrakk\",16,20,1"));
    QVERIFY(line.isValid());
    QCOMPARE(line.type(), QStringLiteral("ENCOUNTER_END"));
}

void LogLineTest::quotedStringWithComma()
{
    QStringList const lines = loadFixtureLines();
    LogLine const encounterStart = findByType(lines, QStringLiteral("ENCOUNTER_START"));
    QVERIFY(encounterStart.isValid());
    // The comma and apostrophes inside the quoted encounter name must
    // survive as a single arg, not split the line into extra args.
    QCOMPARE(encounterStart.argCount(), 6);
    QCOMPARE(encounterStart.argString(2), QStringLiteral("Belo'ren, Child of Al'ar"));
}

void LogLineTest::nestedList()
{
    QStringList const lines = loadFixtureLines();
    LogLine const keyStart = findByType(lines, QStringLiteral("CHALLENGE_MODE_START"));
    QVERIFY(keyStart.isValid());
    QCOMPARE(keyStart.argCount(), 6);

    QVariant const affixesArg = keyStart.arg(5);
    QCOMPARE(affixesArg.typeId(), QMetaType::QVariantList);
    QVariantList const affixes = affixesArg.toList();
    QCOMPARE(affixes.size(), 3);
    QCOMPARE(affixes.at(0).toString(), QStringLiteral("148"));
    QCOMPARE(affixes.at(1).toString(), QStringLiteral("9"));
    QCOMPARE(affixes.at(2).toString(), QStringLiteral("10"));
}

void LogLineTest::timestampParsing()
{
    LogLine line(QStringLiteral("7/27/2024 21:39:13.0951  COMBAT_LOG_VERSION,20"));
    QVERIFY(line.isValid());
    QDateTime const dt = line.dateTime();
    QVERIFY(dt.isValid());
    QCOMPARE(dt.date(), QDate(2024, 7, 27));
    QCOMPARE(dt.time(), QTime(21, 39, 13, 95));
}

void LogLineTest::timestampWithTimezoneOffset()
{
    LogLine line(QStringLiteral("7/11/2026 05:20:02.169-5  COMBAT_LOG_VERSION,22"));
    QVERIFY(line.isValid());
    QDateTime const dt = line.dateTime();
    QVERIFY(dt.isValid());
    QCOMPARE(dt.date(), QDate(2026, 7, 11));
    QCOMPARE(dt.time(), QTime(5, 20, 2, 169));
    QCOMPARE(dt.offsetFromUtc(), -5 * 3600);
}

void LogLineTest::invalidTimestampSeparator()
{
    // Missing the two-space separator between timestamp and payload.
    LogLine line(QStringLiteral("7/27/2024 21:39:13.0951 ENCOUNTER_START,1"));
    QVERIFY(!line.isValid());
}

void LogLineTest::unbalancedBracketIsInvalid()
{
    LogLine line(QStringLiteral("7/27/2024 21:39:13.0951  CHALLENGE_MODE_START,[1,2"));
    QVERIFY(!line.isValid());
}

void LogLineTest::emptyFieldsBetweenCommas()
{
    // Consecutive commas denote an empty/missing field
    LogLine line(QStringLiteral("7/27/2024 21:39:13.0951  SPELL_CAST_SUCCESS,,,,Player-1,\"Foo\""));
    QVERIFY(line.isValid());
    QCOMPARE(line.argCount(), 6);
    QCOMPARE(line.argString(1), QString());
    QCOMPARE(line.argString(2), QString());
    QCOMPARE(line.argString(3), QString());
    QCOMPARE(line.argString(4), QStringLiteral("Player-1"));
    QCOMPARE(line.argString(5), QStringLiteral("Foo"));
}

QTEST_MAIN(LogLineTest)
