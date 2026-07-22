#include "ActivityEncounterDBusAdaptor.h"

#include "ActivityProjection.h"
#include "CurrentActivityObject.h"

using namespace Qt::StringLiterals;

ActivityEncounterDBusAdaptor::ActivityEncounterDBusAdaptor(CurrentActivityObject* activity)
    : QDBusAbstractAdaptor(activity), m_activity(activity)
{
}

uint ActivityEncounterDBusAdaptor::encounterId() const
{
    return constellar::observer::encounterInterfaceProperties(m_activity->bag())
        .value(u"EncounterId"_s)
        .toUInt();
}

QString ActivityEncounterDBusAdaptor::encounterName() const
{
    return constellar::observer::encounterInterfaceProperties(m_activity->bag())
        .value(u"EncounterName"_s)
        .toString();
}

QString ActivityEncounterDBusAdaptor::difficulty() const
{
    return constellar::observer::encounterInterfaceProperties(m_activity->bag())
        .value(u"Difficulty"_s)
        .toString();
}

uint ActivityEncounterDBusAdaptor::difficultyId() const
{
    return constellar::observer::encounterInterfaceProperties(m_activity->bag())
        .value(u"DifficultyId"_s)
        .toUInt();
}
