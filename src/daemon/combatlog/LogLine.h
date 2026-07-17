#pragma once

#include <QDateTime>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVector>

// A parsed line from the WoW combat log.
//
// Grammar: TIMESTAMP + "  " (two spaces) + EVENT "," ARG "," ARG ...
// Args can nest via [...] or (...)class LogLine
class LogLine
{
  public:
    explicit LogLine(QString rawLine);

    bool isValid() const
    {
        return m_valid;
    }

    QString const& raw() const
    {
        return m_raw;
    }

    QString const& rawTimestamp() const
    {
        return m_timestamp;
    }

    QDateTime dateTime() const;

    // e.g. ENCOUNTER_START
    QString type() const;

    int argCount() const
    {
        return m_args.size();
    }

    // Either a QString or QVariantList for nested [...] groups
    QVariant arg(int index) const;

    QString argString(int index) const;

  private:
    void parse();

    QString m_raw;
    QString m_timestamp;
    QVector<QVariant> m_args;
    bool m_valid = false;
};
