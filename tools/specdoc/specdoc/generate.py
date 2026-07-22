"""CLI entry point for specdoc."""

from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path

from .dbus import parse_interfaces
from .html import (
    check_index_coverage,
    check_interface_coverage,
    interface_filename,
    render_index,
    render_interface,
)
from .keys import render_keys_header
from .spec import cross_check, load_spec

_REPO_ROOT = Path(__file__).resolve().parents[3]
_DEFAULT_SPEC = _REPO_ROOT / "data" / "dev.ulduar.Constellar1.spec.yaml"
_DEFAULT_XML = _REPO_ROOT / "data" / "dev.ulduar.Constellar1.xml"
_DEFAULT_KEYS_HEADER = _REPO_ROOT / "src" / "common" / "ActivityKeys.h"
_DEFAULT_TEMPLATES = _REPO_ROOT / "tools" / "specdoc" / "templates"
_DEFAULT_DOC_OUT = _REPO_ROOT / "doc" / "spec"
_DEFAULT_LICENSE = _REPO_ROOT / "LICENSE"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="specdoc")
    parser.add_argument(
        "--emit", choices=["keys", "html"], default="keys", help="What to generate."
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
        help="--emit keys only: don't write --out; fail if it differs from freshly "
        "generated output, or if the sidecar and XML disagree.",
    )
    parser.add_argument(
        "--templates",
        type=Path,
        default=_DEFAULT_TEMPLATES,
        help="--emit html only: Jinja template directory.",
    )
    parser.add_argument(
        "--doc-out",
        type=Path,
        default=_DEFAULT_DOC_OUT,
        help="--emit html only: output directory for index.html, the per-interface "
        "pages, and style.css.",
    )
    parser.add_argument(
        "--license",
        type=Path,
        default=_DEFAULT_LICENSE,
        help="--emit html only: LICENSE file whose full text is embedded in the "
        "landing page's legal section.",
    )
    args = parser.parse_args(argv)

    spec = load_spec(args.spec)

    cross_check_errors = cross_check(spec, args.xml)
    for err in cross_check_errors:
        print(f"specdoc: cross-check error: {err}", file=sys.stderr)

    if args.emit == "keys":
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

    # --emit html
    interfaces = parse_interfaces(args.xml)

    license_text = args.license.read_text(encoding="utf-8")
    index_html = render_index(spec, interfaces, args.templates, license_text)
    pages = {"index.html": index_html}

    coverage_errors = check_index_coverage(index_html, interfaces)
    for iface in interfaces.values():
        page = render_interface(spec, iface, args.templates)
        pages[interface_filename(iface.name)] = page
        coverage_errors += check_interface_coverage(page, spec, iface)

    for err in coverage_errors:
        print(f"specdoc: coverage error: {err}", file=sys.stderr)

    if cross_check_errors or coverage_errors:
        return 1

    args.doc_out.mkdir(parents=True, exist_ok=True)
    for filename, content in pages.items():
        (args.doc_out / filename).write_text(content, encoding="utf-8")
    shutil.copy(args.templates / "style.css", args.doc_out / "style.css")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
