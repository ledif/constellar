#pragma once

// Terminal result of an activity, surfaced on the Observer `outcome` a{sv} key.
// Encounters use Success/Failure/Abandoned; dungeons use Success/Abandoned only
// (the combat log cannot distinguish a timed run from a depleted one).
enum class ActivityOutcome
{
    Unknown,
    Success,
    Failure,
    Abandoned,
};
