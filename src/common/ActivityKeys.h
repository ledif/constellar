#pragma once

// Key vocabulary for the Activity/Zone a{sv} metadata
namespace constellar::keys
{

// Activity metadata
inline constexpr auto kType = "type";
inline constexpr auto kEncounterId = "encounterId";
inline constexpr auto kEncounterName = "encounterName";
inline constexpr auto kDifficulty = "difficulty";
inline constexpr auto kDifficultyId = "difficultyId";
inline constexpr auto kMapId = "mapId";
inline constexpr auto kZoneId = "zoneId";
inline constexpr auto kKeystoneLevel = "keystoneLevel";
inline constexpr auto kStartTime = "startTime";

// ActivityEnded-only additions
inline constexpr auto kSuccess = "success";
inline constexpr auto kDurationMs = "durationMs";
inline constexpr auto kStopTime = "stopTime";

// Zone stuff
inline constexpr auto kZoneName = "zoneName";

// kType ("type") values
inline constexpr auto kTypeEncounter = "encounter";
inline constexpr auto kTypeDungeon = "dungeon";

}  // namespace constellar::keys
