#include "ActivityDBusAdaptor.h"

#include "ActivityProjection.h"
#include "dbus/ActivityObject.h"

using namespace Qt::StringLiterals;

ActivityDBusAdaptor::ActivityDBusAdaptor(ActivityObject* activity)
    : QDBusAbstractAdaptor(activity), m_activity(activity)
{
}

QString ActivityDBusAdaptor::type() const
{
    return constellar::observer::activityInterfaceProperties(m_activity->bag())
        .value(u"Type"_s)
        .toString();
}

qint64 ActivityDBusAdaptor::startTime() const
{
    return constellar::observer::activityInterfaceProperties(m_activity->bag())
        .value(u"StartTime"_s)
        .toLongLong();
}

qint64 ActivityDBusAdaptor::stopTime() const
{
    return constellar::observer::activityInterfaceProperties(m_activity->bag())
        .value(u"StopTime"_s)
        .toLongLong();
}

QString ActivityDBusAdaptor::outcome() const
{
    return constellar::observer::activityInterfaceProperties(m_activity->bag())
        .value(u"Outcome"_s)
        .toString();
}

QDBusObjectPath ActivityDBusAdaptor::recording() const
{
    return constellar::observer::activityInterfaceProperties(m_activity->bag())
        .value(u"Recording"_s)
        .value<QDBusObjectPath>();
}
