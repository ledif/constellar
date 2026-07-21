#include "Location.h"

#include <QDBusArgument>
#include <QList>

#include "ActivityKeys.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

namespace
{

// In-process, the wire map holds a QList<double> directly. After a round trip through
// D-Bus, the a{sv} value arrives client-side as an undemarshalled QDBusArgument — decode
// both shapes.
QList<double> boundsFromVariant(QVariant const& value)
{
    if (value.canConvert<QDBusArgument>())
    {
        QList<double> bounds;
        value.value<QDBusArgument>() >> bounds;
        return bounds;
    }

    return value.value<QList<double>>();
}

}  // namespace

QPointF MapBounds::normalize(double worldX, double worldY) const
{
    if (!isValid())
        return {};

    return QPointF((worldX - x0) / (x1 - x0), (worldY - y0) / (y1 - y0));
}

Location Location::fromVariantMap(QVariantMap const& map)
{
    Location location;

    if (map.contains(QString::fromLatin1(keys::kUiMapId)))
    {
        UiMap uiMap;
        uiMap.id = map.value(QString::fromLatin1(keys::kUiMapId)).toUInt();
        uiMap.name = map.value(QString::fromLatin1(keys::kUiMapName)).toString();

        QList<double> const bounds =
            boundsFromVariant(map.value(QString::fromLatin1(keys::kUiMapBounds)));
        if (bounds.size() == 4)
            uiMap.bounds = MapBounds{bounds[0], bounds[1], bounds[2], bounds[3]};

        location.m_uiMap = uiMap;
    }

    if (map.contains(QString::fromLatin1(keys::kZoneInstanceId)))
    {
        Zone zone;
        zone.instanceId = map.value(QString::fromLatin1(keys::kZoneInstanceId)).toUInt();
        zone.name = map.value(QString::fromLatin1(keys::kZoneName)).toString();
        zone.difficultyId = map.value(QString::fromLatin1(keys::kZoneDifficultyId)).toUInt();

        location.m_zone = zone;
    }

    return location;
}

QVariantMap Location::toVariantMap() const
{
    QVariantMap map;

    if (m_uiMap)
    {
        map[QString::fromLatin1(keys::kUiMapId)] = m_uiMap->id;
        map[QString::fromLatin1(keys::kUiMapName)] = m_uiMap->name;
        if (m_uiMap->bounds.isValid())
        {
            map[QString::fromLatin1(keys::kUiMapBounds)] = QVariant::fromValue(
                QList<double>{
                    m_uiMap->bounds.x0, m_uiMap->bounds.x1, m_uiMap->bounds.y0, m_uiMap->bounds.y1
                }
            );
        }
    }

    if (m_zone)
    {
        map[QString::fromLatin1(keys::kZoneInstanceId)] = m_zone->instanceId;
        map[QString::fromLatin1(keys::kZoneName)] = m_zone->name;
        map[QString::fromLatin1(keys::kZoneDifficultyId)] = m_zone->difficultyId;
    }

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
