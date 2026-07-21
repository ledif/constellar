#pragma once

#include <QPointF>
#include <QString>
#include <QVariantMap>
#include <optional>

// World-coordinate bounding box from MAP_CHANGE args 3..6, in raw arg order.
struct MapBounds
{
    double x0 = 0, x1 = 0, y0 = 0, y1 = 0;
    bool isValid() const
    {
        return x0 != x1 && y0 != y1;
    }
    // world (x,y) -> normalized 0..1. NOTE: axis/sign pairing is UNVERIFIED — leave
    // this here but do not rely on its output until checked against a known position.
    QPointF normalize(double worldX, double worldY) const;
};

// The cartographic map (C_Map / UiMap system) — MAP_CHANGE.
struct UiMap
{
    quint32 id = 0;  // uiMapID
    QString name;
    MapBounds bounds;
};

// The named area/instance the player stands in — ZONE_CHANGE.
struct Zone
{
    quint32 instanceId = 0;    // Map.dbc id; 0 in open world
    QString name;              // real zone text
    quint32 difficultyId = 0;  // 0 when not instanced
};

// Ambient location: two independently-updated halves. Neither invalidates the other.
class Location
{
  public:
    static Location fromVariantMap(QVariantMap const& map);
    QVariantMap toVariantMap() const;

    std::optional<UiMap> const& uiMap() const
    {
        return m_uiMap;
    }
    std::optional<Zone> const& zone() const
    {
        return m_zone;
    }
    void setUiMap(UiMap const& m)
    {
        m_uiMap = m;
    }
    void setZone(Zone const& z)
    {
        m_zone = z;
    }

    bool isEmpty() const
    {
        return !m_uiMap && !m_zone;
    }
    // Presence display: prefer the granular area, fall back to the map.
    QString displayName() const;  // zone.name, else uiMap.name, else {}
    QString toString() const;     // displayName() or u"none"

  private:
    std::optional<UiMap> m_uiMap;
    std::optional<Zone> m_zone;
};
