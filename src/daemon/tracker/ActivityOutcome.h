#pragma once

// Encounters use Success/Failure/Abandoned;
// Dungeons use Success/Abandoned only
enum class ActivityOutcome
{
    Unknown,
    Success,
    Failure,
    Abandoned,
};
