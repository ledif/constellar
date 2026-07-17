#pragma once

#include <QDateTime>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVector>

// A parsed line from the WoW combat log.
//
// Grammar: TIMESTAMP + "  " (two spaces) + EVENT "," ARG "," ARG ...
// Args can nest via [...] or (...) (e.g. affix arrays, spell lists) and can
// be quoted strings containing commas. See PLAN.md §6.
//
// We only support the TWW+ timestamp format that includes the year
// ("7/27/2024 21:39:13.0951") — the pre-TWW format is out of scope.
//
// v1 parses all args eagerly on construction; the reference implementation
// parses lazily as an optimization for huge lines like COMBATANT_INFO, but
// that's not required for correctness (PLAN.md §3.3).
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

    // Parsed from rawTimestamp(); invalid QDateTime if the timestamp didn't
    // match the expected format.
    QDateTime dateTime() const;

    // Convenience for arg(0), the event name (e.g. "ENCOUNTER_START").
    QString type() const;

    int argCount() const
    {
        return m_args.size();
    }

    // Returns an invalid QVariant if index is out of range. A value is
    // either a QString or, for nested [...]/(...)  groups, a QVariantList.
    QVariant arg(int index) const;

    // Convenience: arg(index).toString().
    QString argString(int index) const;

  private:
    void parse();

    QString m_raw;
    QString m_timestamp;
    QVector<QVariant> m_args;
    bool m_valid = false;
};
