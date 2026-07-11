#pragma once

#include <QObject>

class RecordingControllerTest : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    void startsRecordingAboveThreshold();
    void skipsBelowThreshold();
    void skipsUnknownDifficulty();
    void stopsAfterOverrunDelay();
    void repullDuringOverrunEndsPreviousImmediately();
    void ignoresNonRaidLines();
    void ignoresStrayEncounterEndWithoutStart();
    void ignoresMismatchedEncounterEnd();
};
