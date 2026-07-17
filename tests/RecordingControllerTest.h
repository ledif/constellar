#pragma once

#include <QObject>

class RecordingControllerTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void startsRecordingAboveThreshold();
    void skipsBelowThreshold();
    void skipsUnknownDifficulty();
    void stopsAfterOverrunDelay();
    void repullDuringOverrunEndsPreviousImmediately();
    void ignoresUnhandledLines();
    void ignoresStrayEncounterEndWithoutStart();
    void ignoresMismatchedEncounterEnd();

    void dungeonStartsAboveKeystoneThreshold();
    void dungeonSkipsBelowKeystoneThreshold();
    void dungeonStopsAfterOverrunDelay();
    void dungeonRepullDuringOverrunEndsPreviousImmediately();
    void dungeonSuppressesNestedEncounterSignals();
    void dungeonIgnoresReStartWhileStillActive();

    void mapChangeEmitsZoneChanged();
};
