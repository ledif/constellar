#pragma once

#include <QObject>

class InotifyEventParserTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void parsesSingleEvent();
    void parsesMultipleEventsInOneBuffer();
    void parsesEventWithNoName();
    void rejectsBufferShorterThanHeader();
    void rejectsTruncatedName();
    void stopsAtBoundaryBetweenTwoReads();
    void boundsNameByLenWhenNoNulTerminatorPresent();
};
