#!/bin/bash
# Replays a combat log fixture for constellard
#
# Usage: scripts/replay-log.sh <log-dir> [speed] [fixture]

set -euo pipefail

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

log_dir=${1:?usage: $0 <log-dir> [speed] [fixture]}
speed=${2:-20}
fixture=${3:-"$script_dir/fixtures/midnight-season1-alt-raid.tsv"}

mkdir -p "$log_dir"
log_file="$log_dir/WoWCombatLog.txt"
touch "$log_file"

ts() { date '+%-m/%-d/%Y %H:%M:%S.000-0'; }

echo "Replaying $fixture into $log_file at ${speed}x speed"

while IFS=$'\t' read -r delta event; do
  [[ -z "$delta" || "$delta" == \#* ]] && continue

  sleep_for=$(awk -v d="$delta" -v s="$speed" 'BEGIN { t = d / s; print (t < 0 ? 0 : t) }')
  sleep "$sleep_for"

  echo "$(ts)  $event" >>"$log_file"
  echo "+${delta}s (slept ${sleep_for}s) -> $event"
done <"$fixture"

echo "Replay complete."
