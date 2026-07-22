#include "ActivityDungeonDBusAdaptor.h"

#include "ActivityProjection.h"
#include "CurrentActivityObject.h"

using namespace Qt::StringLiterals;

ActivityDungeonDBusAdaptor::ActivityDungeonDBusAdaptor(CurrentActivityObject* activity)
    : QDBusAbstractAdaptor(activity), m_activity(activity)
{
}

uint ActivityDungeonDBusAdaptor::zoneId() const
{
    return constellar::observer::dungeonInterfaceProperties(m_activity->bag())
        .value(u"ZoneId"_s)
        .toUInt();
}

uint ActivityDungeonDBusAdaptor::keystoneLevel() const
{
    return constellar::observer::dungeonInterfaceProperties(m_activity->bag())
        .value(u"KeystoneLevel"_s)
        .toUInt();
}

uint ActivityDungeonDBusAdaptor::challengeMapId() const
{
    return constellar::observer::dungeonInterfaceProperties(m_activity->bag())
        .value(u"ChallengeMapId"_s)
        .toUInt();
}

qint64 ActivityDungeonDBusAdaptor::durationMs() const
{
    return constellar::observer::dungeonInterfaceProperties(m_activity->bag())
        .value(u"DurationMs"_s)
        .toLongLong();
}
