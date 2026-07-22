#pragma once

#include <QString>
#include <QVariantMap>
#include <optional>

// This is the name of the map you see when you press m
//   (e.g., Silvermoon City)
struct UiMap
{
    quint32 id = 0;  // uiMapID
    QString name;
};

// This is the name of the zone within the map you're in
//   (e.g., The Bazaar)
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
