#pragma once

// GENERATED from data/dev.ulduar.Constellar1.spec.yaml

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
inline constexpr auto kOutcome = "outcome";
inline constexpr auto kDurationMs = "durationMs";
inline constexpr auto kStopTime = "stopTime";

// Location stuff
inline constexpr auto kZoneName = "zoneName";
inline constexpr auto kZoneCategory = "zoneCategory";
inline constexpr auto kUiMapId = "uiMapId";

// kType ("type") values
inline constexpr auto kTypeEncounter = "encounter";
inline constexpr auto kTypeDungeon = "dungeon";
inline constexpr auto kTypeUnknown = "unknown";

// kOutcome ("outcome") values
inline constexpr auto kOutcomeUnknown = "unknown";
inline constexpr auto kOutcomeSuccess = "success";
inline constexpr auto kOutcomeFailure = "failure";
inline constexpr auto kOutcomeAbandoned = "abandoned";

}  // namespace constellar::keys
