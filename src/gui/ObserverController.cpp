#include "ObserverController.h"

#include <KFormat>
#include <QDateTime>

#include "Activity.h"
#include "ActivityKeys.h"
#include "DBusConstants.h"
#include "Location.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

namespace
{

QString formatDuration(qint64 ms)
{
    if (ms < 0)
        ms = 0;

    qint64 const totalSeconds = ms / 1000;
    qint64 const hours = totalSeconds / 3600;
    qint64 const minutes = (totalSeconds % 3600) / 60;
    qint64 const seconds = totalSeconds % 60;

    if (hours > 0)
    {
        return u"%1:%2:%3"_s.arg(hours)
            .arg(minutes, 2, 10, QChar(u'0'))
            .arg(seconds, 2, 10, QChar(u'0'));
    }

    return u"%1:%2"_s.arg(minutes).arg(seconds, 2, 10, QChar(u'0'));
}

}  // namespace

ObserverController::ObserverController(QObject* parent)
    : QObject(parent),
      m_manager(
          constellar::dbus::kServiceName, constellar::dbus::kObjectPath,
          QDBusConnection::sessionBus()
      ),
      m_pollTimer(this),
      m_eventLog(this),
      m_now(QDateTime::currentMSecsSinceEpoch())
{
    connect(&m_manager, &ObserverProxy::ActivityEnded, this, &ObserverController::onActivityEnded);

    connect(&m_pollTimer, &QTimer::timeout, this, &ObserverController::refresh);
    m_pollTimer.start(1000);
    refresh();
}

void ObserverController::refresh()
{
    m_now = QDateTime::currentMSecsSinceEpoch();
    Q_EMIT nowChanged();

    if (!m_manager.isValid())
    {
        m_zoneText.clear();
        Q_EMIT stateChanged();
        return;
    }

    QVariantMap const activityMap = m_manager.property("Activity").toMap();
    Activity const activity = Activity::fromVariantMap(activityMap);
    Location const location = Location::fromVariantMap(m_manager.property("Location").toMap());

    if (m_previousActivity.isEmpty() && !activityMap.isEmpty())
    {
        qint64 const startTime =
            activityMap.value(QString::fromLatin1(keys::kStartTime)).toLongLong();
        m_eventLog.beginActivity(activity.toString(), startTime);
    }
    m_previousActivity = activityMap;

    m_zoneText = location.displayName();
    Q_EMIT stateChanged();
}

void ObserverController::onActivityEnded(
    [[maybe_unused]] QDBusObjectPath const& path, QVariantMap const& activityMap
)
{
    Activity const activity = Activity::fromVariantMap(activityMap);
    qint64 const startTime = activityMap.value(QString::fromLatin1(keys::kStartTime)).toLongLong();
    QString const outcome = activityMap.value(QString::fromLatin1(keys::kOutcome)).toString();
    bool const success = outcome == QString::fromLatin1(keys::kOutcomeSuccess);
    qint64 const stopTime = activityMap.value(QString::fromLatin1(keys::kStopTime)).toLongLong();
    qint64 const durationMs =
        activityMap.value(QString::fromLatin1(keys::kDurationMs)).toLongLong();

    m_eventLog.endActivity(activity.toString(), startTime, success, stopTime, durationMs);
    m_previousActivity.clear();
}

QString ObserverController::relativeTime(qint64 epochMs) const
{
    return KFormat().formatRelativeDateTime(
        QDateTime::fromMSecsSinceEpoch(epochMs), QLocale::ShortFormat
    );
}

QString ObserverController::elapsed(qint64 startMs) const
{
    return formatDuration(m_now - startMs);
}

QString ObserverController::durationText(qint64 durationMs) const
{
    return formatDuration(durationMs);
}
