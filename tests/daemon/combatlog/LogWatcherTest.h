#pragma once

#include <QObject>

class LogWatcherTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void tailsNewWrites();
    void handlesPartialLineAtEof();
    void resetsOffsetOnFileRecreation();
    void ignoresNonCombatLogFiles();
    void emitsIdleTimeoutAfterInactivity();
    void tailsMultipleCombatLogFiles();
};
