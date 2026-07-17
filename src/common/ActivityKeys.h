#pragma once

// Key vocabulary for the Activity/Zone a{sv} fact bags carried by
// io.github.ledif.constellar.Observer (see data/io.github.ledif.constellar.xml
// and ADR-012). Source of truth for key names so ObserverService,
// PresencePublisher, and the CLI never hand-type string literals that can
// drift out of sync.
namespace constellar::keys {

// Activity bag (also carried by the ActivityEnded signal).
inline constexpr auto kType = "type";
inline constexpr auto kEncounterId = "encounterId";
inline constexpr auto kEncounterName = "encounterName";
inline constexpr auto kDifficulty = "difficulty";
inline constexpr auto kDifficultyId = "difficultyId";
inline constexpr auto kMapId = "mapId";
inline constexpr auto kZoneId = "zoneId";
inline constexpr auto kKeystoneLevel = "keystoneLevel";
inline constexpr auto kStartTime = "startTime";

// ActivityEnded-only additions.
inline constexpr auto kSuccess = "success";
inline constexpr auto kDurationMs = "durationMs";
inline constexpr auto kStopTime = "stopTime";

// Zone bag.
inline constexpr auto kZoneName = "zoneName";

// Activity "type" values.
inline constexpr auto kTypeEncounter = "encounter";
inline constexpr auto kTypeDungeon = "dungeon";

}  // namespace constellar::keys
