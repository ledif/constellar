"""CLI entry point for specdoc."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from .keys import render_keys_header
from .spec import cross_check, load_spec

_REPO_ROOT = Path(__file__).resolve().parents[3]
_DEFAULT_SPEC = _REPO_ROOT / "data" / "io.github.ledif.constellar.spec.yaml"
_DEFAULT_XML = _REPO_ROOT / "data" / "io.github.ledif.constellar.xml"
_DEFAULT_KEYS_HEADER = _REPO_ROOT / "src" / "common" / "ActivityKeys.h"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="specdoc")
    parser.add_argument(
        "--emit", choices=["keys"], default="keys", help="What to generate."
    )
    parser.add_argument("--spec", type=Path, default=_DEFAULT_SPEC)
    parser.add_argument("--xml", type=Path, default=_DEFAULT_XML)
    parser.add_argument(
        "--out",
        type=Path,
        default=_DEFAULT_KEYS_HEADER,
        help="Where to write the generated header (--emit keys).",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Don't write --out; fail if it differs from freshly generated output, "
        "or if the sidecar and XML a{sv} properties disagree.",
    )
    args = parser.parse_args(argv)

    spec = load_spec(args.spec)

    cross_check_errors = cross_check(spec, args.xml)
    for err in cross_check_errors:
        print(f"specdoc: cross-check error: {err}", file=sys.stderr)

    generated = render_keys_header(spec)

    if args.check:
        current = args.out.read_text(encoding="utf-8") if args.out.exists() else None
        drifted = current != generated
        if drifted:
            print(
                f"specdoc: {args.out} is stale",
                file=sys.stderr,
            )
        if drifted or cross_check_errors:
            return 1
        return 0

    if cross_check_errors:
        return 1

    args.out.write_text(generated, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
