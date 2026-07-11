#include "LogLineTest.h"

#include <QFile>
#include <QTest>
#include <QTextStream>
#include <QVariantList>

#include "LogLine.h"

namespace {

// Fixture is hand-written per the grammar in PLAN.md §6, not a captured
// real combat log (none were available at scaffold time) — swap in real
// trimmed logs here per PLAN.md §6 once available.
QStringList loadFixtureLines() {
    QFile file(QStringLiteral(LOGFIXTURES_DIR "/sample.txt"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    QStringList lines;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    }
    return lines;
}

LogLine findByType(const QStringList &lines, const QString &type) {
    for (const QString &raw : lines) {
        LogLine line(raw);
        if (line.isValid() && line.type() == type) {
            return line;
        }
    }
    return LogLine(QString());
}

}  // namespace

void LogLineTest::basicFields() {
    const QStringList lines = loadFixtureLines();
    QVERIFY(!lines.isEmpty());

    const LogLine encounterStart = findByType(lines, QStringLiteral("ENCOUNTER_START"));
    QVERIFY(encounterStart.isValid());
    QCOMPARE(encounterStart.argCount(), 6);
    QCOMPARE(encounterStart.argString(1), QStringLiteral("2820"));
    QCOMPARE(encounterStart.argString(2), QStringLiteral("Fyrakk the Blazing"));
    QCOMPARE(encounterStart.argString(3), QStringLiteral("16"));
    QCOMPARE(encounterStart.argString(4), QStringLiteral("20"));
    QCOMPARE(encounterStart.argString(5), QStringLiteral("2549"));
}

void LogLineTest::type() {
    LogLine line(QStringLiteral("7/27/2024 21:39:13.0951  ENCOUNTER_END,2820,\"Fyrakk\",16,20,1"));
    QVERIFY(line.isValid());
    QCOMPARE(line.type(), QStringLiteral("ENCOUNTER_END"));
}

void LogLineTest::quotedStringWithComma() {
    const QStringList lines = loadFixtureLines();
    const LogLine zoneChange = findByType(lines, QStringLiteral("ZONE_CHANGE"));
    QVERIFY(zoneChange.isValid());
    // The comma and apostrophe inside the quoted zone name must survive as
    // a single arg, not split the line into extra args.
    QCOMPARE(zoneChange.argCount(), 4);
    QCOMPARE(zoneChange.argString(2), QStringLiteral("Amirdrassil, the Dream's Hope"));
}

void LogLineTest::nestedList() {
    const QStringList lines = loadFixtureLines();
    const LogLine keyStart = findByType(lines, QStringLiteral("CHALLENGE_MODE_START"));
    QVERIFY(keyStart.isValid());
    QCOMPARE(keyStart.argCount(), 6);

    const QVariant affixesArg = keyStart.arg(5);
    QCOMPARE(affixesArg.typeId(), QMetaType::QVariantList);
    const QVariantList affixes = affixesArg.toList();
    QCOMPARE(affixes.size(), 4);
    QCOMPARE(affixes.at(0).toString(), QStringLiteral("9"));
    QCOMPARE(affixes.at(1).toString(), QStringLiteral("152"));
    QCOMPARE(affixes.at(2).toString(), QStringLiteral("158"));
    QCOMPARE(affixes.at(3).toString(), QStringLiteral("0"));
}

void LogLineTest::timestampParsing() {
    LogLine line(QStringLiteral("7/27/2024 21:39:13.0951  COMBAT_LOG_VERSION,20"));
    QVERIFY(line.isValid());
    const QDateTime dt = line.dateTime();
    QVERIFY(dt.isValid());
    QCOMPARE(dt.date(), QDate(2024, 7, 27));
    QCOMPARE(dt.time(), QTime(21, 39, 13, 95));
}

void LogLineTest::timestampWithTimezoneOffset() {
    // Real combat logs append a timezone offset with no separator, e.g.
    // ".169-5" — not documented in PLAN.md §6, discovered testing against a
    // live client. Must not be swallowed into the millisecond fraction.
    LogLine line(QStringLiteral("7/11/2026 05:20:02.169-5  COMBAT_LOG_VERSION,22"));
    QVERIFY(line.isValid());
    const QDateTime dt = line.dateTime();
    QVERIFY(dt.isValid());
    QCOMPARE(dt.date(), QDate(2026, 7, 11));
    QCOMPARE(dt.time(), QTime(5, 20, 2, 169));
    QCOMPARE(dt.offsetFromUtc(), -5 * 3600);
}

void LogLineTest::invalidTimestampSeparator() {
    // Missing the two-space separator between timestamp and payload.
    LogLine line(QStringLiteral("7/27/2024 21:39:13.0951 ENCOUNTER_START,1"));
    QVERIFY(!line.isValid());
}

void LogLineTest::unbalancedBracketIsInvalid() {
    LogLine line(QStringLiteral("7/27/2024 21:39:13.0951  CHALLENGE_MODE_START,[1,2"));
    QVERIFY(!line.isValid());
}

void LogLineTest::emptyFieldsBetweenCommas() {
    // Consecutive commas denote an empty/missing field, common for optional
    // GUID-type args; make sure they don't get collapsed.
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
