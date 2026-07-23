#!/usr/bin/env python3
"""Golden-reference D-Bus contract test.

Pins the behavior and properties for /dev/ulduar/Constellar1 objects

Usage: contract.py <constellard-path>

Set CONSTELLAR_UPDATE_GOLDENS=1 to write the goldens instead of diffing
"""

import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
import xml.etree.ElementTree as ET
from pathlib import Path

SERVICE = "dev.ulduar.Constellar1"
ROOT_PATH = "/dev/ulduar/Constellar1"
ACTIVITY_PATH = "/dev/ulduar/Constellar1/activity/current"

SCRIPT_DIR = Path(__file__).resolve().parent
GOLDEN_DIR = SCRIPT_DIR / "goldens"

UPDATE_GOLDENS = bool(os.environ.get("CONSTELLAR_UPDATE_GOLDENS"))


class Failure(Exception):
    pass


class Daemon:
    def __init__(self, constellard: str):
        self.constellard = constellard
        self.log_dir: Path | None = None
        self.log_file = None
        self.proc: subprocess.Popen | None = None

    def start(self):
        self.log_dir = Path(tempfile.mkdtemp())
        self.log_file = tempfile.TemporaryFile(mode="w+")
        self.proc = subprocess.Popen(
            [self.constellard, "--log-dir", str(self.log_dir)],
            stdout=self.log_file,
            stderr=subprocess.STDOUT,
        )

        print(f"Waiting for constellard to start watching {self.log_dir}...")
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            if self.proc.poll() is not None:
                self.log_file.seek(0)
                sys.stderr.write("constellard exited before it started watching:\n")
                sys.stderr.write(self.log_file.read())
                raise Failure("constellard exited early")

            self.log_file.seek(0)
            if "running, watching" in self.log_file.read():
                return
            time.sleep(0.2)

        raise Failure("constellard never reported it was watching")

    def write_log_lines(self, lines: list[str]):
        (self.log_dir / "WoWCombatLog.txt").write_text("\n".join(lines) + "\n")

    def stop(self):
        if self.proc is not None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()
            self.proc = None
        if self.log_dir is not None:
            shutil.rmtree(self.log_dir, ignore_errors=True)
            self.log_dir = None
        if self.log_file is not None:
            self.log_file.close()
            self.log_file = None


def gdbus(*args: str) -> subprocess.CompletedProcess:
    return subprocess.run(
        ["gdbus", *args], capture_output=True, text=True, check=False
    )


def wait_for_activity_object():
    output = ""
    for _ in range(50):
        result = gdbus(
            "call",
            "--session",
            "-d",
            SERVICE,
            "-o",
            ROOT_PATH,
            "-m",
            "org.freedesktop.DBus.ObjectManager.GetManagedObjects",
        )
        output = result.stdout
        if f"'{ACTIVITY_PATH}'" in output:
            return
        time.sleep(0.2)
    raise Failure(f"/activity/current never appeared in GetManagedObjects: {output}")


def normalize_xml(raw: str) -> str:
    # gdbus prefixes a DOCTYPE preamble that canonicalize() chokes on.
    data = raw[raw.index("<node") :]
    return ET.canonicalize(xml_data=data)


DYNAMIC_FIELD = re.compile(r"'(StartTime|StopTime)': <int64 -?\d+>")


def normalize_dynamic(text: str) -> str:
    return DYNAMIC_FIELD.sub(lambda m: f"'{m.group(1)}': <int64 <DYNAMIC>>", text)


def check_golden(name: str, content: str):
    path = GOLDEN_DIR / name
    if UPDATE_GOLDENS:
        GOLDEN_DIR.mkdir(parents=True, exist_ok=True)
        path.write_text(content + "\n")
        print(f"updated golden: {name}")
        return

    if not path.exists():
        raise Failure(
            f"missing golden {path} (run with CONSTELLAR_UPDATE_GOLDENS=1 to generate)"
        )

    expected = path.read_text().rstrip("\n")
    if expected != content:
        raise Failure(
            f"{name} does not match the committed golden\n"
            f"--- expected ({path}) ---\n{expected}\n"
            f"--- actual ---\n{content}"
        )
    print(f"OK: {name} matches golden")


def assert_error(desc: str, expected_error: str, args: list[str]):
    result = gdbus(*args)
    combined = result.stdout + result.stderr
    if result.returncode == 0:
        raise Failure(
            f"{desc}: expected failure containing {expected_error}, "
            f"but call succeeded: {combined}"
        )
    if expected_error not in combined:
        raise Failure(f"{desc}: expected error containing {expected_error}, got: {combined}")
    print(f"OK: {desc} -> {expected_error}")


def properties_args(method: str, path: str, *rest: str) -> list[str]:
    return [
        "call",
        "--session",
        "-d",
        SERVICE,
        "-o",
        path,
        "-m",
        f"org.freedesktop.DBus.Properties.{method}",
        *rest,
    ]


def main():
    if len(sys.argv) < 2:
        print("usage: contract.py <constellard-path>", file=sys.stderr)
        return 2

    constellard = sys.argv[1]
    daemon = Daemon(constellard)
    try:
        daemon.start()

        # Test case (1): there should be no activity live yet
        assert_error(
            "Properties call with no activity live",
            "org.freedesktop.DBus.Error.UnknownObject",
            properties_args("GetAll", ACTIVITY_PATH, "dev.ulduar.Constellar1.Activity"),
        )

        # Encounter scenario (from smoke.sh).
        daemon.write_log_lines(
            [
                "7/16/2026 18:57:18.869-5  MAP_CHANGE,2533,\"March on Quel'Danas\","
                "10956.250000,10152.083008,-4002.083984,-5208.333984",
                "7/15/2026 18:42:33.566-5  ENCOUNTER_START,3182,"
                "\"Belo'ren, Child of Al'ar\",14,11,2913",
            ]
        )

        wait_for_activity_object()

        introspect_xml = normalize_xml(
            gdbus("introspect", "--session", "-d", SERVICE, "-o", ACTIVITY_PATH, "--xml").stdout
        )

        check_golden("activity-introspect-encounter.xml", introspect_xml)

        activity_getall = normalize_dynamic(
            gdbus(
                *properties_args("GetAll", ACTIVITY_PATH, "dev.ulduar.Constellar1.Activity")
            ).stdout.strip()
        )

        check_golden("activity-getall-encounter.txt", activity_getall)

        encounter_getall = gdbus(
            *properties_args(
                "GetAll", ACTIVITY_PATH, "dev.ulduar.Constellar1.Activity.Encounter"
            )
        ).stdout.strip()

        check_golden("encounter-getall.txt", encounter_getall)

        # Test case (2): Get of a nonexistent property on a live interface.
        assert_error(
            "Get of unknown property",
            "org.freedesktop.DBus.Error.UnknownProperty",
            properties_args(
                "Get",
                ACTIVITY_PATH,
                "dev.ulduar.Constellar1.Activity.Encounter",
                "NoSuchProperty",
            ),
        )

        # Test case (3): GetAll on .Activity.Dungeon while an encounter is live.
        assert_error(
            "GetAll on Dungeon interface during an encounter",
            "org.freedesktop.DBus.Error.UnknownInterface",
            properties_args("GetAll", ACTIVITY_PATH, "dev.ulduar.Constellar1.Activity.Dungeon"),
        )

        # Error case (4): Set on a live read-only property.
        assert_error(
            "Set of a read-only property",
            "org.freedesktop.DBus.Error.PropertyReadOnly",
            properties_args(
                "Set", ACTIVITY_PATH, "dev.ulduar.Constellar1.Activity", "Type", "<'bogus'>"
            ),
        )

        daemon.stop()

        # Second round of tests (M+)
        daemon = Daemon(constellard)
        daemon.start()

        daemon.write_log_lines(
            [
                "7/13/2026 21:05:38.080-5  CHALLENGE_MODE_START,"
                "\"Magisters' Terrace\",2811,558,10,[148,9,10]",
            ]
        )

        wait_for_activity_object()

        introspect_xml = normalize_xml(
            gdbus("introspect", "--session", "-d", SERVICE, "-o", ACTIVITY_PATH, "--xml").stdout
        )

        check_golden("activity-introspect-dungeon.xml", introspect_xml)

        dungeon_getall = gdbus(
            *properties_args("GetAll", ACTIVITY_PATH, "dev.ulduar.Constellar1.Activity.Dungeon")
        ).stdout.strip()

        check_golden("dungeon-getall.txt", dungeon_getall)

        daemon.stop()

        print("OK")
        return 0
    except Failure as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    finally:
        daemon.stop()


if __name__ == "__main__":
    sys.exit(main())
