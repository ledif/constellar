#include "InotifyEventParserTest.h"

#include <cstddef>
#include <span>

#include <sys/inotify.h>

#include <QTest>

#include "InotifyEventParser.h"

using constellar::inotify::nextEvent;

namespace
{

QByteArray headerBytes(quint32 mask, quint32 len)
{
    struct inotify_event header{};
    header.wd = 1;
    header.mask = mask;
    header.cookie = 0;
    header.len = len;
    return QByteArray(reinterpret_cast<char const*>(&header), sizeof(header));
}

std::span<std::byte const> asSpan(QByteArray const& bytes)
{
    return std::span<std::byte const>(
        reinterpret_cast<std::byte const*>(bytes.constData()),
        static_cast<std::size_t>(bytes.size())
    );
}

}  // namespace

void InotifyEventParserTest::parsesSingleEvent()
{
    QByteArray nameField("WoWCombatLog.txt");
    nameField.append('\0');

    QByteArray const buffer =
        headerBytes(IN_CREATE, static_cast<quint32>(nameField.size())) + nameField;

    auto const result = nextEvent(asSpan(buffer));
    QVERIFY(result.has_value());
    QCOMPARE(result->first.mask, static_cast<quint32>(IN_CREATE));
    QCOMPARE(result->first.name, QStringLiteral("WoWCombatLog.txt"));
    QVERIFY(result->second.empty());
}

void InotifyEventParserTest::parsesMultipleEventsInOneBuffer()
{
    QByteArray firstName("a.txt");
    firstName.append('\0');
    QByteArray secondName("b.txt");
    secondName.append('\0');

    QByteArray buffer = headerBytes(IN_CREATE, static_cast<quint32>(firstName.size())) + firstName;
    buffer += headerBytes(IN_MODIFY, static_cast<quint32>(secondName.size())) + secondName;

    auto const first = nextEvent(asSpan(buffer));
    QVERIFY(first.has_value());
    QCOMPARE(first->first.mask, static_cast<quint32>(IN_CREATE));
    QCOMPARE(first->first.name, QStringLiteral("a.txt"));

    auto const second = nextEvent(first->second);
    QVERIFY(second.has_value());
    QCOMPARE(second->first.mask, static_cast<quint32>(IN_MODIFY));
    QCOMPARE(second->first.name, QStringLiteral("b.txt"));
    QVERIFY(second->second.empty());
}

void InotifyEventParserTest::parsesEventWithNoName()
{
    // Self-events on the watched directory itself (e.g. IN_IGNORED) carry no name
    QByteArray const buffer = headerBytes(IN_IGNORED, 0);

    auto const result = nextEvent(asSpan(buffer));
    QVERIFY(result.has_value());
    QCOMPARE(result->first.mask, static_cast<quint32>(IN_IGNORED));
    QVERIFY(result->first.name.isEmpty());
    QVERIFY(result->second.empty());
}

void InotifyEventParserTest::rejectsBufferShorterThanHeader()
{
    QVERIFY(!nextEvent(std::span<std::byte const>()).has_value());

    QByteArray const partialHeader(sizeof(struct inotify_event) - 1, '\0');
    QVERIFY(!nextEvent(asSpan(partialHeader)).has_value());
}

void InotifyEventParserTest::rejectsTruncatedName()
{
    // Header claims a 20-byte name, but none of it is actually present.
    QByteArray const buffer = headerBytes(IN_MODIFY, 20);
    QVERIFY(!nextEvent(asSpan(buffer)).has_value());
}

void InotifyEventParserTest::stopsAtBoundaryBetweenTwoReads()
{
    QByteArray completeName("a.txt");
    completeName.append('\0');
    QByteArray buffer =
        headerBytes(IN_CREATE, static_cast<quint32>(completeName.size())) + completeName;

    // A second event whose header is present but whose name got cut off
    buffer += headerBytes(IN_MODIFY, 20);

    auto const first = nextEvent(asSpan(buffer));
    QVERIFY(first.has_value());
    QCOMPARE(first->first.name, QStringLiteral("a.txt"));

    QVERIFY(!nextEvent(first->second).has_value());
}

void InotifyEventParserTest::boundsNameByLenWhenNoNulTerminatorPresent()
{
    // no \0 terminator in the name field
    QByteArray const nameField(16, 'A');
    QByteArray const buffer =
        headerBytes(IN_MODIFY, static_cast<quint32>(nameField.size())) + nameField;

    auto const result = nextEvent(asSpan(buffer));
    QVERIFY(result.has_value());
    QCOMPARE(result->first.name, QString::fromLatin1(nameField));
    QVERIFY(result->second.empty());
}

QTEST_MAIN(InotifyEventParserTest)
