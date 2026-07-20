#pragma once

#include <QObject>

class GameStateTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void setActivityEmitsOnChange();
    void setActivitySkipsOnIdenticalBag();
    void setZoneEmitsOnChange();
    void setZoneSkipsOnIdenticalBag();
    void endActivityEmitsThenClears();
};
