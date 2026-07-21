#pragma once

#include <QString>
#include <QVariantMap>
#include <optional>

// Cartographic map from MAP_CHANGE events.
struct UiMap
{
    quint32 id = 0;  // uiMapID
    QString name;
};

// Named area / instance from ZONE_CHANGE events.
struct Zone
{
    QString name;
    quint32 difficultyId = 0;  // raw ZONE_CHANGE difficultyID
};

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

    QString displayName() const;
    QString toString() const;

  private:
    std::optional<UiMap> m_uiMap;
    std::optional<Zone> m_zone;
};
