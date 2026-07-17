#pragma once

#include <QString>
#include <QVariantMap>

// Client-side view of the Zone fact bag carried by ObserverProxy's Zone
// property (see ActivityKeys.h). Parses the a{sv} wire format once so the
// CLI and GUI don't each hand-roll their own QVariantMap parsing and
// display-string formatting.
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

    // "March on Quel'Danas (mapId 2214)", or "none".
    QString toDisplayString() const;

  private:
    QString m_name;
    uint m_mapId = 0;
};
