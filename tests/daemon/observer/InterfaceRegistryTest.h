#pragma once

#include <QObject>

class InterfaceRegistryTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void parsesActivityInterface();
    void parsesEncounterInterface();
    void parsesDungeonInterface();
    void unknownInterfaceIsNull();
};
