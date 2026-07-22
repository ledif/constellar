#include "Location.h"

#include "ActivityKeys.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

namespace
{

QString zoneCategory(quint32 difficultyId)
{
    switch (difficultyId)
    {
        case 0:
            return u"open-world"_s;
        case 1:   // Dungeon Normal
        case 2:   // Dungeon Heroic
        case 23:  // Dungeon Mythic
        case 24:  // Timewalking
            return u"dungeon"_s;
        case 14:  // Raid Normal
        case 15:  // Raid Heroic
        case 16:  // Raid Mythic
        case 17:  // Raid LFR
            return u"raid"_s;
        case 208:  // Delve (constant across every delve and every tier)
            return u"delve"_s;
        default:
            return u"unknown"_s;
    }
}

}  // namespace

Location Location::fromVariantMap(QVariantMap const& map)
{
    Location location;

    if (map.contains(QString::fromLatin1(keys::kUiMapId)))
    {
        UiMap uiMap;
        uiMap.id = map.value(QString::fromLatin1(keys::kUiMapId)).toUInt();
        location.m_uiMap = uiMap;
    }

    if (map.contains(QString::fromLatin1(keys::kZoneName)))
    {
        Zone zone;
        zone.name = map.value(QString::fromLatin1(keys::kZoneName)).toString();
        location.m_zone = zone;
    }

    return location;
}

QVariantMap Location::toVariantMap() const
{
    QVariantMap map;

    if (isEmpty())
        return map;

    map[QString::fromLatin1(keys::kZoneName)] = displayName();
    map[QString::fromLatin1(keys::kZoneCategory)] =
        m_zone ? zoneCategory(m_zone->difficultyId) : u"unknown"_s;
    map[QString::fromLatin1(keys::kUiMapId)] = m_uiMap ? m_uiMap->id : 0u;

    return map;
}

QString Location::displayName() const
{
    if (m_zone && !m_zone->name.isEmpty())
        return m_zone->name;

    if (m_uiMap)
        return m_uiMap->name;

    return {};
}

QString Location::toString() const
{
    QString const name = displayName();
    return name.isEmpty() ? u"none"_s : name;
}
