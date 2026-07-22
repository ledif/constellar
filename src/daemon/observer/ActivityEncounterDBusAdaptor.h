#pragma once

#include <QDBusAbstractAdaptor>
#include <QString>

#include "DBusConstants.h"

class CurrentActivityObject;

// dev.ulduar.Constellar1.Activity.Encounter -- present only when the current
// activity is an encounter (TASK-008).
class ActivityEncounterDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", CONSTELLAR_ACTIVITY_ENCOUNTER_INTERFACE_NAME)

    Q_PROPERTY(uint EncounterId READ encounterId)
    Q_PROPERTY(QString EncounterName READ encounterName)
    Q_PROPERTY(QString Difficulty READ difficulty)
    Q_PROPERTY(uint DifficultyId READ difficultyId)

  public:
    explicit ActivityEncounterDBusAdaptor(CurrentActivityObject* activity);

    uint encounterId() const;
    QString encounterName() const;
    QString difficulty() const;
    uint difficultyId() const;

  private:
    CurrentActivityObject* m_activity;
};
