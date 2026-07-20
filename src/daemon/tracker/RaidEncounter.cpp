#include "RaidEncounter.h"

namespace
{

constexpr int kDifficultyIdLFR = 17;
constexpr int kDifficultyIdNormal = 14;
constexpr int kDifficultyIdHeroic = 15;
constexpr int kDifficultyIdMythic = 16;

}  // namespace

std::optional<RaidDifficulty> raidDifficultyFromId(int difficultyId)
{
    switch (difficultyId)
    {
        case kDifficultyIdLFR:
            return RaidDifficulty::LFR;
        case kDifficultyIdNormal:
            return RaidDifficulty::Normal;
        case kDifficultyIdHeroic:
            return RaidDifficulty::Heroic;
        case kDifficultyIdMythic:
            return RaidDifficulty::Mythic;
        default:
            return std::nullopt;
    }
}
