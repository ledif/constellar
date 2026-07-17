#pragma once

#include <QString>
#include <QVariantMap>

enum class ActivityType
{
    None,
    Encounter,
    Dungeon,
};

// Client-side view of the Activity exposed on the wire
class Activity
{
  public:
    static Activity fromVariantMap(QVariantMap const& map);

    ActivityType type() const
    {
        return m_type;
    }

    bool isNone() const
    {
        return m_type == ActivityType::None;
    }

    QString difficulty() const
    {
        return m_difficulty;
    }

    QString encounterName() const
    {
        return m_encounterName;
    }

    uint keystoneLevel() const
    {
        return m_keystoneLevel;
    }

    QString toString() const;

  private:
    ActivityType m_type = ActivityType::None;
    QString m_difficulty;
    QString m_encounterName;
    uint m_keystoneLevel = 0;
};
