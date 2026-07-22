#pragma once

#include <QVariantMap>

// Projects the same Activity bag GameState holds (see ActivityMetadata) onto the
// typed dev.ulduar.Constellar1.Activity(.Encounter|.Dungeon) object properties
// (RFC-006 / TASK-008). One source of truth: the bag, read the same way the
// Observer.Activity a{sv} property already is.
namespace constellar::observer
{

bool isEncounter(QVariantMap const& bag);
bool isDungeon(QVariantMap const& bag);

// dev.ulduar.Constellar1.Activity properties. Recording is always the `/`
// sentinel until TASK-010 gives it something to point at.
QVariantMap activityInterfaceProperties(QVariantMap const& bag);

// dev.ulduar.Constellar1.Activity.Encounter properties. Only meaningful when
// isEncounter(bag).
QVariantMap encounterInterfaceProperties(QVariantMap const& bag);

// dev.ulduar.Constellar1.Activity.Dungeon properties. Only meaningful when
// isDungeon(bag).
QVariantMap dungeonInterfaceProperties(QVariantMap const& bag);

}  // namespace constellar::observer
