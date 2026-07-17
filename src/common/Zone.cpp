#include "Zone.h"

#include "ActivityKeys.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

Zone Zone::fromVariantMap(QVariantMap const& map)
{
    Zone zone;
    zone.m_name = map.value(QString::fromLatin1(keys::kZoneName)).toString();
    zone.m_mapId = map.value(QString::fromLatin1(keys::kMapId)).toUInt();
    return zone;
}

QString Zone::toString() const
{
    if (isEmpty())
        return u"none"_s;

    return m_name;
}
