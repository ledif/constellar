#pragma once

#include <QObject>

class ActivityTrackerTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void startsRecordingAboveThreshold();
    void skipsBelowThreshold();
    void skipsUnknownDifficulty();
    void stopsAfterOverrunDelay();
    void repullDuringOverrunEndsPreviousImmediately();
    void encounterOverlapWithoutEndWipesPrevious();
    void encounterEndSuccessFalseIsRecorded();
    void ignoresUnhandledLines();
    void ignoresStrayEncounterEndWithoutStart();
    void ignoresMismatchedEncounterEnd();

    void dungeonStartsAboveKeystoneThreshold();
    void dungeonSkipsBelowKeystoneThreshold();
    void dungeonStopsAfterOverrunDelay();
    void dungeonRepullDuringOverrunEndsPreviousImmediately();
    void dungeonSuppressesNestedEncounterSignals();
    void dungeonIgnoresReStartWhileStillActive();
    void dungeonEndEndsActiveKeyRegardlessOfArgs();

    void mapChangeEmitsUiMapChanged();
    void mapChangeIgnoresBoundsColumns();
    void zoneChangeEmitsZoneChanged();
    void zoneChangeWithoutMapChangeDoesNotEmitUiMapChanged();
};
