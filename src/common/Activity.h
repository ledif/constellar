#pragma once

#include <QString>
#include <QVariantMap>

// Client-side view of the Activity fact bag carried by ObserverProxy's
// Activity property / ActivityEnded signal (see ActivityKeys.h). Parses the
// a{sv} wire format once so the CLI and GUI don't each hand-roll their own
// QVariantMap parsing and display-string formatting.
class Activity
{
  public:
    enum class Type
    {
        None,
        Encounter,
        Dungeon,
    };

    static Activity fromVariantMap(QVariantMap const& map);

    Type type() const
    {
        return m_type;
    }
    bool isNone() const
    {
        return m_type == Type::None;
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

    // Human-readable summary: "Mythic Ulgrax the Devourer", "Mythic+ 18", or
    // "none".
    QString toDisplayString() const;

  private:
    Type m_type = Type::None;
    QString m_difficulty;
    QString m_encounterName;
    uint m_keystoneLevel = 0;
};
