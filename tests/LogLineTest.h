#pragma once

#include <QObject>

class LogLineTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void basicFields();
    void type();
    void quotedStringWithComma();
    void nestedList();
    void timestampParsing();
    void timestampWithTimezoneOffset();
    void invalidTimestampSeparator();
    void unbalancedBracketIsInvalid();
    void emptyFieldsBetweenCommas();
};
