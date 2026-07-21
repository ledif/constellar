#pragma once

#include <QJsonObject>
#include <QPointF>
#include <QString>
#include <QVariantMap>
#include <optional>

// World-coordinates from MAP_CHANGE events
struct MapBounds
{
    double x0 = 0, x1 = 0, y0 = 0, y1 = 0;
    bool isValid() const
    {
        return x0 != x1 && y0 != y1;
    }
};

struct UiMap
{
    quint32 id = 0;  // uiMapID
    QString name;
    MapBounds bounds;
};

// named area/instance from ZONE_CHANGE events
struct Zone
{
    quint32 instanceId = 0;
    QString name;
    quint32 difficultyId = 0;
};

class Location
{
  public:
    static Location fromVariantMap(QVariantMap const& map);
    QVariantMap toVariantMap() const;
    QJsonObject toJsonObject() const;

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

    QString displayName() const;
    QString toString() const;

  private:
    std::optional<UiMap> m_uiMap;
    std::optional<Zone> m_zone;
};
