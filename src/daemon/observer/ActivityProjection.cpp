#include "ActivityProjection.h"

#include <QDBusObjectPath>

#include "ActivityKeys.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

namespace
{

QString keyStr(char const* key)
{
    return QString::fromLatin1(key);
}

QString bagString(QVariantMap const& bag, char const* key, char const* fallback)
{
    QString const value = bag.value(keyStr(key)).toString();
    return value.isEmpty() ? QString::fromLatin1(fallback) : value;
}

}  // namespace

namespace constellar::observer
{

bool isEncounter(QVariantMap const& bag)
{
    return bag.value(keyStr(keys::kType)).toString() == QString::fromLatin1(keys::kTypeEncounter);
}

bool isDungeon(QVariantMap const& bag)
{
    return bag.value(keyStr(keys::kType)).toString() == QString::fromLatin1(keys::kTypeDungeon);
}

QVariantMap activityInterfaceProperties(QVariantMap const& bag)
{
    QVariantMap props;
    props[u"Type"_s] = bagString(bag, keys::kType, keys::kTypeUnknown);
    props[u"StartTime"_s] = bag.value(keyStr(keys::kStartTime)).toLongLong();
    props[u"StopTime"_s] = bag.value(keyStr(keys::kStopTime)).toLongLong();
    props[u"Outcome"_s] = bagString(bag, keys::kOutcome, keys::kOutcomeUnknown);
    props[u"Recording"_s] = QVariant::fromValue(QDBusObjectPath(u"/"_s));
    return props;
}

QVariantMap encounterInterfaceProperties(QVariantMap const& bag)
{
    QVariantMap props;
    props[u"EncounterId"_s] = bag.value(keyStr(keys::kEncounterId)).toUInt();
    props[u"EncounterName"_s] = bag.value(keyStr(keys::kEncounterName)).toString();
    props[u"Difficulty"_s] = bag.value(keyStr(keys::kDifficulty)).toString();
    props[u"DifficultyId"_s] = bag.value(keyStr(keys::kDifficultyId)).toUInt();
    return props;
}

QVariantMap dungeonInterfaceProperties(QVariantMap const& bag)
{
    QVariantMap props;
    props[u"ZoneId"_s] = bag.value(keyStr(keys::kZoneId)).toUInt();
    props[u"KeystoneLevel"_s] = bag.value(keyStr(keys::kKeystoneLevel)).toUInt();
    props[u"ChallengeMapId"_s] = bag.value(keyStr(keys::kChallengeMapId)).toUInt();
    props[u"DurationMs"_s] = bag.value(keyStr(keys::kDurationMs)).toLongLong();
    return props;
}

}  // namespace constellar::observer
