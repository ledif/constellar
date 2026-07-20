#pragma once

#include <QObject>

class ObserverServiceTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void encounterStartPopulatesActivity();
    void mapChangePopulatesZone();
    void encounterEndClearsActivity();
    void dungeonStartPopulatesActivity();
};
