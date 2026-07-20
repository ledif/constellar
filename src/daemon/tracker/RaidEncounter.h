#pragma once

#include <optional>

#include <QDateTime>
#include <QString>

enum class RaidDifficulty
{
    LFR = 0,
    Normal = 1,
    Heroic = 2,
    Mythic = 3
};

std::optional<RaidDifficulty> raidDifficultyFromId(int difficultyId);

struct RaidEncounter
{
    int encounterId = 0;
    QString encounterName;
    int difficultyId = 0;
    QDateTime startTime;
};
