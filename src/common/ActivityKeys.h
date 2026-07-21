#pragma once

// GENERATED from data/io.github.ledif.constellar.spec.yaml

// Key vocabulary for the Activity/Zone a{sv} metadata
namespace constellar::keys
{

// Activity metadata
inline constexpr auto kType = "type";
inline constexpr auto kEncounterId = "encounterId";
inline constexpr auto kEncounterName = "encounterName";
inline constexpr auto kDifficulty = "difficulty";
inline constexpr auto kDifficultyId = "difficultyId";
inline constexpr auto kZoneId = "zoneId";
inline constexpr auto kKeystoneLevel = "keystoneLevel";
inline constexpr auto kStartTime = "startTime";
inline constexpr auto kChallengeMapId = "challengeMapId";

// ActivityEnded-only additions
inline constexpr auto kSuccess = "success";
inline constexpr auto kDurationMs = "durationMs";
inline constexpr auto kStopTime = "stopTime";

// Location stuff
inline constexpr auto kUiMapId = "uiMapId";
inline constexpr auto kUiMapName = "uiMapName";
inline constexpr auto kUiMapBounds = "uiMapBounds";
inline constexpr auto kZoneInstanceId = "zoneInstanceId";
inline constexpr auto kZoneName = "zoneName";
inline constexpr auto kZoneDifficultyId = "zoneDifficultyId";

// kType ("type") values
inline constexpr auto kTypeEncounter = "encounter";
inline constexpr auto kTypeDungeon = "dungeon";

}  // namespace constellar::keys
