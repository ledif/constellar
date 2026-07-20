#include "RaidEncounter.h"

std::optional<RaidDifficulty> raidDifficultyFromId(int difficultyId)
{
    switch (difficultyId)
    {
        case 17:
            return RaidDifficulty::LFR;
        case 14:
            return RaidDifficulty::Normal;
        case 15:
            return RaidDifficulty::Heroic;
        case 16:
            return RaidDifficulty::Mythic;
        default:
            return std::nullopt;
    }
}
