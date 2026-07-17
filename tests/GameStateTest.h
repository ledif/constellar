#pragma once

#include <QObject>

// Covers GameState's compare-and-skip setters and the activityEnded edge
// (ADR-012 / TASK-001).
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
