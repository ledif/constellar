#pragma once

#include <QVariantMap>

namespace constellar::observer
{

bool isEncounter(QVariantMap const& bag);
bool isDungeon(QVariantMap const& bag);

// dev.ulduar.Constellar1.Activity properties
QVariantMap activityInterfaceProperties(QVariantMap const& bag);

// dev.ulduar.Constellar1.Activity.Encounter properties
QVariantMap encounterInterfaceProperties(QVariantMap const& bag);

// dev.ulduar.Constellar1.Activity.Dungeon properties
QVariantMap dungeonInterfaceProperties(QVariantMap const& bag);

}  // namespace constellar::observer
