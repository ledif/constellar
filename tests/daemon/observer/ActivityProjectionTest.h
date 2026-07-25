#pragma once

#include <QObject>

class ActivityProjectionTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void encounterProjectsActivityAndEncounterInterfaces();
    void dungeonProjectsActivityAndDungeonInterfaces();
    void unknownTypeDefaultsAreUnknown();
    void recordingIsAlwaysTheNoObjectSentinel();
};
