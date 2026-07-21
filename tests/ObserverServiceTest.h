#pragma once

#include <QObject>

class ObserverServiceTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void encounterStartPopulatesActivity();
    void mapChangePopulatesLocation();
    void zoneChangePopulatesLocation();
    void encounterEndClearsActivity();
    void dungeonStartPopulatesActivity();
};
