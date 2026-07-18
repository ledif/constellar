#include "ObserverController.h"

#include <QDateTime>

#include "Activity.h"
#include "ActivityKeys.h"
#include "DBusConstants.h"
#include "Zone.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

ObserverController::ObserverController(QObject* parent)
    : QObject(parent),
      m_manager(
          constellar::dbus::kServiceName, constellar::dbus::kObjectPath,
          QDBusConnection::sessionBus()
      ),
      m_pollTimer(this),
      m_eventLog(this),
      m_activityText(u"connecting..."_s)
{
    connect(&m_manager, &ObserverProxy::ActivityEnded, this, &ObserverController::onActivityEnded);

    connect(&m_pollTimer, &QTimer::timeout, this, &ObserverController::refresh);
    m_pollTimer.start(1000);
    refresh();
}

void ObserverController::refresh()
{
    if (!m_manager.isValid())
    {
        m_activityText = u"constellard not reachable"_s;
        m_zoneText.clear();
        Q_EMIT stateChanged();
        return;
    }

    QVariantMap const activityMap = m_manager.property("Activity").toMap();
    Activity const activity = Activity::fromVariantMap(activityMap);
    Zone const zone = Zone::fromVariantMap(m_manager.property("Zone").toMap());

    if (m_previousActivity.isEmpty() && !activityMap.isEmpty())
    {
        m_eventLog.addEntry(
            u"▶ %1"_s.arg(activity.toString()), true, QDateTime::currentMSecsSinceEpoch()
        );
    }
    m_previousActivity = activityMap;

    m_activityText = activity.toString();
    m_zoneText = zone.name();
    Q_EMIT stateChanged();
}

QString ObserverController::shortTime(qint64 epochMs)
{
    return QDateTime::fromMSecsSinceEpoch(epochMs).toString(u"HH:mm:ss"_s);
}

void ObserverController::onActivityEnded(QVariantMap const& activityMap)
{
    Activity const activity = Activity::fromVariantMap(activityMap);
    bool const success = activityMap.value(QString::fromLatin1(keys::kSuccess)).toBool();
    qint64 const stopTime = activityMap.value(QString::fromLatin1(keys::kStopTime)).toLongLong();
    m_eventLog.addEntry(
        u"■ %1 — %2"_s.arg(activity.toString(), shortTime(stopTime)), success, stopTime
    );
    m_previousActivity.clear();
}
