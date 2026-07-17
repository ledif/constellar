#pragma once

#include <QString>
#include <QVariantMap>

// Client-side view of the zone fact exposed on the wire
class Zone
{
  public:
    static Zone fromVariantMap(QVariantMap const& map);

    bool isEmpty() const
    {
        return m_name.isEmpty();
    }
    QString name() const
    {
        return m_name;
    }
    uint mapId() const
    {
        return m_mapId;
    }

    QString toString() const;

  private:
    QString m_name;
    uint m_mapId = 0;
};
