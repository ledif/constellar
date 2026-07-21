#pragma once

#include <QObject>

class GameStateTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void setActivityEmitsOnChange();
    void setActivitySkipsOnIdenticalBag();
    void setUiMapEmitsOnChange();
    void setUiMapSkipsOnIdenticalValue();
    void setZoneEmitsOnChange();
    void setZoneSkipsOnIdenticalValue();
    void setUiMapThenSetZoneMergesBothHalves();
    void oneHalfUpdateDoesNotClearTheOther();
    void endActivityEmitsThenClears();
};
