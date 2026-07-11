#include "LogLine.h"

#include <QRegularExpression>
#include <QTimeZone>
#include <QVector>

LogLine::LogLine(QString rawLine) : m_raw(std::move(rawLine)) {
    parse();
}

QDateTime LogLine::dateTime() const {
    // "7/11/2026 05:20:02.169-5" -> month, day, year, hour, min, sec,
    // fraction, timezone offset (hours, optionally ":mm"). The offset suffix
    // isn't in PLAN.md's grammar spec but is present on every line of a real
    // combat log — without it we'd silently parse the wrong instant.
    static const QRegularExpression pattern(QStringLiteral(
        R"(^(\d{1,2})/(\d{1,2})/(\d{4})\s+(\d{1,2}):(\d{2}):(\d{2})\.(\d+)([+-]\d{1,2}(?::\d{2})?)?$)"));

    const QRegularExpressionMatch match = pattern.match(m_timestamp);
    if (!match.hasMatch()) {
        return {};
    }

    const int month = match.captured(1).toInt();
    const int day = match.captured(2).toInt();
    const int year = match.captured(3).toInt();
    const int hour = match.captured(4).toInt();
    const int minute = match.captured(5).toInt();
    const int second = match.captured(6).toInt();

    QString fraction = match.captured(7).left(3);
    while (fraction.size() < 3) {
        fraction += QLatin1Char('0');
    }
    const int millisecond = fraction.toInt();

    const QDate date(year, month, day);
    const QTime time(hour, minute, second, millisecond);
    if (!date.isValid() || !time.isValid()) {
        return {};
    }

    const QString offsetStr = match.captured(8);
    if (offsetStr.isEmpty()) {
        return QDateTime(date, time);
    }

    const bool negative = offsetStr.startsWith(QLatin1Char('-'));
    const QStringList parts = offsetStr.mid(1).split(QLatin1Char(':'));
    const int offsetHours = parts.value(0).toInt();
    const int offsetMinutes = parts.value(1, QStringLiteral("0")).toInt();
    int offsetSeconds = offsetHours * 3600 + offsetMinutes * 60;
    if (negative) {
        offsetSeconds = -offsetSeconds;
    }

    return QDateTime(date, time, QTimeZone::fromSecondsAheadOfUtc(offsetSeconds));
}

QString LogLine::type() const {
    return argString(0);
}

QVariant LogLine::arg(int index) const {
    if (index < 0 || index >= m_args.size()) {
        return {};
    }
    return m_args.at(index);
}

QString LogLine::argString(int index) const {
    return arg(index).toString();
}

namespace {

// A pending value being accumulated: either plain text, or (once a
// [...]/(...)  group has just closed) a completed QVariantList waiting to be
// committed into its enclosing scope by the next delimiter. Mirrors the
// reference LogLine.ts parser's `value` variable, which is deliberately
// generic for the same reason.
bool isSet(const QVariant &value) {
    return value.typeId() == QMetaType::QVariantList || !value.toString().isEmpty();
}

}  // namespace

void LogLine::parse() {
    const int sep = m_raw.indexOf(QStringLiteral("  "));
    if (sep < 0) {
        m_valid = false;
        return;
    }

    m_timestamp = m_raw.left(sep);

    QVector<QVariantList> openLists;
    bool inQuotedString = false;
    QVariant value = QString();

    auto commit = [&](const QVariant &committed) {
        if (!openLists.isEmpty()) {
            openLists.last().append(committed);
        } else {
            m_args.append(committed);
        }
    };

    const int length = m_raw.length();
    for (int pos = sep + 2; pos < length; ++pos) {
        const QChar c = m_raw.at(pos);
        if (c == QLatin1Char('\n')) {
            break;
        }

        if (inQuotedString) {
            if (c == QLatin1Char('"')) {
                inQuotedString = false;
            } else {
                value = value.toString() + c;
            }
            continue;
        }

        if (c == QLatin1Char(',')) {
            commit(value);
            value = QString();
        } else if (c == QLatin1Char('"')) {
            inQuotedString = true;
        } else if (c == QLatin1Char('[') || c == QLatin1Char('(')) {
            openLists.append(QVariantList());
        } else if (c == QLatin1Char(']') || c == QLatin1Char(')')) {
            if (openLists.isEmpty()) {
                // Unbalanced closer; treat the line as malformed rather than
                // throwing, since the daemon should stay up on a bad line.
                m_valid = false;
                return;
            }
            if (isSet(value)) {
                openLists.last().append(value);
            }
            value = openLists.takeLast();
        } else {
            value = value.toString() + c;
        }
    }

    if (isSet(value)) {
        commit(value);
    }

    m_valid = openLists.isEmpty() && !m_args.isEmpty();
}
