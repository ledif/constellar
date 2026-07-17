#include "Activity.h"

#include "ActivityKeys.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

Activity Activity::fromVariantMap(QVariantMap const& map)
{
    QString const type = map.value(QString::fromLatin1(keys::kType)).toString();

    Activity activity;
    if (type == QString::fromLatin1(keys::kTypeEncounter))
    {
        activity.m_type = Type::Encounter;
        activity.m_difficulty = map.value(QString::fromLatin1(keys::kDifficulty)).toString();
        activity.m_encounterName = map.value(QString::fromLatin1(keys::kEncounterName)).toString();
    }
    else if (type == QString::fromLatin1(keys::kTypeDungeon))
    {
        activity.m_type = Type::Dungeon;
        activity.m_keystoneLevel = map.value(QString::fromLatin1(keys::kKeystoneLevel)).toUInt();
    }
    return activity;
}

QString Activity::toString() const
{
    switch (m_type)
    {
        case Type::Encounter:
            return u"%1 %2"_s.arg(m_difficulty, m_encounterName);
        case Type::Dungeon:
            return u"Mythic+ %1"_s.arg(m_keystoneLevel);
        case Type::None:
            return u"none"_s;
    }
    return u"none"_s;
}
