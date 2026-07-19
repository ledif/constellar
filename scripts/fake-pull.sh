#!/bin/bash
# Appends a synthetic raid pull (ENCOUNTER_START/END) to a combat-log file so
# a running `constellard` (see `just run-daemon <dir>`) picks it up, letting
# you exercise the GUI's activity row end-to-end without a real WoW client.
#
# Usage: scripts/fake-pull.sh <log-dir> [pull-seconds] [success(0|1)]
#
# <log-dir> must be the same directory passed to `just run-daemon`. Difficulty
# is hardcoded to Normal (14) since the daemon's default minDifficulty rejects
# LFR. After ENCOUNTER_END, constellard waits raidOverrunSeconds (20s by
# default) before actually emitting ActivityEnded over DBus — the GUI row
# stays in the "in progress" state for that long after the fight ends.

set -euo pipefail

log_dir=${1:?usage: $0 <log-dir> [pull-seconds] [success(0|1)]}
pull_seconds=${2:-10}
success=${3:-1}

mkdir -p "$log_dir"
log_file="$log_dir/WoWCombatLog.txt"
touch "$log_file"

encounter_id=3159
encounter_name="Rotmire"
difficulty_id=14   # Normal
group_size=20
instance_id=1592

ts() { date '+%-m/%-d/%Y %H:%M:%S.000-0'; }

echo "$(ts)  ENCOUNTER_START,$encounter_id,\"$encounter_name\",$difficulty_id,$group_size,$instance_id" >> "$log_file"
echo "Wrote ENCOUNTER_START to $log_file — pull running for ${pull_seconds}s"

sleep "$pull_seconds"

duration_ms=$((pull_seconds * 1000))
echo "$(ts)  ENCOUNTER_END,$encounter_id,\"$encounter_name\",$difficulty_id,$group_size,$success,$duration_ms" >> "$log_file"
echo "Wrote ENCOUNTER_END (success=$success) to $log_file"
echo "constellard applies a raidOverrunSeconds delay (default 20s) before ActivityEnded fires."
