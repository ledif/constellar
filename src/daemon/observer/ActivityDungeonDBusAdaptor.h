#pragma once

#include <QDBusAbstractAdaptor>

#include "DBusConstants.h"

class CurrentActivityObject;

// dev.ulduar.Constellar1.Activity.Dungeon -- present only when the current
// activity is a dungeon (TASK-008).
class ActivityDungeonDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", CONSTELLAR_ACTIVITY_DUNGEON_INTERFACE_NAME)

    Q_PROPERTY(uint ZoneId READ zoneId)
    Q_PROPERTY(uint KeystoneLevel READ keystoneLevel)
    Q_PROPERTY(uint ChallengeMapId READ challengeMapId)
    Q_PROPERTY(qint64 DurationMs READ durationMs)

  public:
    explicit ActivityDungeonDBusAdaptor(CurrentActivityObject* activity);

    uint zoneId() const;
    uint keystoneLevel() const;
    uint challengeMapId() const;
    qint64 durationMs() const;

  private:
    CurrentActivityObject* m_activity;
};
