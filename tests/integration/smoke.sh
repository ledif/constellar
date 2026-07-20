#!/bin/bash
# Integration round-trip
set -euo pipefail

if [ "$#" -lt 2 ]; then
  echo "usage: smoke.sh <constellard-path> <constellarctl-path>" >&2
  exit 2
fi

constellard="$1"
constellarctl="$2"

log_dir=$(mktemp -d)
daemon_log=$(mktemp)
daemon_pid=""

cleanup() {
  if [ -n "$daemon_pid" ]; then
    kill "$daemon_pid" >/dev/null 2>&1 || true
    wait "$daemon_pid" 2>/dev/null || true
  fi
  rm -rf "$log_dir" "$daemon_log"
}
trap cleanup EXIT

"$constellard" --log-dir "$log_dir" >"$daemon_log" 2>&1 &
daemon_pid=$!

echo "Waiting for constellard to start watching $log_dir..."
until grep -q "running, watching" "$daemon_log" 2>/dev/null; do
  if ! kill -0 "$daemon_pid" 2>/dev/null; then
    echo "constellard exited before it started watching:" >&2
    cat "$daemon_log" >&2
    exit 1
  fi
  sleep 0.2
done

# Above-threshold (Normal) encounter with no END: the activity stays
# populated, so there's no overrun wait to poll for.
printf '%s\n' \
  '7/15/2026 18:42:33.566-5  ENCOUNTER_START,3182,"Belo'"'"'ren, Child of Al'"'"'ar",14,11,2913' \
  >"$log_dir/WoWCombatLog.txt"

status_json=""
for _ in $(seq 1 50); do
  status_json=$("$constellarctl" status --json 2>/dev/null || true)
  if echo "$status_json" | grep -q '"type":"encounter"'; then
    break
  fi
  sleep 0.2
done

echo "constellarctl status --json: $status_json"

if ! echo "$status_json" | grep -q '"type":"encounter"'; then
  echo "FAIL: expected activity type \"encounter\", got: $status_json" >&2
  exit 1
fi

if ! echo "$status_json" | grep -q '"encounterName":"Belo'"'"'ren, Child of Al'"'"'ar"'; then
  echo "FAIL: expected encounterName \"Belo'ren, Child of Al'ar\", got: $status_json" >&2
  exit 1
fi

if ! echo "$status_json" | grep -q '"difficulty":"Normal"'; then
  echo "FAIL: expected difficulty \"Normal\", got: $status_json" >&2
  exit 1
fi

echo "OK"
