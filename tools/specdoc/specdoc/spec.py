"""Loading and validation of the Observer spec sidecar."""

from __future__ import annotations

import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path

import yaml


@dataclass
class Spec:
    raw: dict
    path: Path

    @property
    def bags(self) -> dict:
        return self.raw.get("bags", {})


def load_spec(path: Path) -> Spec:
    with path.open("r", encoding="utf-8") as f:
        raw = yaml.safe_load(f)
    return Spec(raw=raw, path=path)


def _xml_avsv_properties(xml_path: Path) -> set[str]:
    """Names of properties in the XML declared as an `a{sv}` envelope."""
    tree = ET.parse(xml_path)
    root = tree.getroot()
    names = set()
    for interface in root.findall("interface"):
        for prop in interface.findall("property"):
            if prop.get("type") == "a{sv}":
                names.add(prop.get("name"))
    return names


def cross_check(spec: Spec, xml_path: Path) -> list[str]:
    """Return a list of human-readable errors. Empty means no errors."""
    errors = []
    xml_props = _xml_avsv_properties(xml_path)
    sidecar_bags = set(spec.bags.keys())

    for bag in sidecar_bags - xml_props:
        errors.append(
            f"sidecar documents bag '{bag}' but the XML has no matching a{{sv}} property"
        )
    for prop in xml_props - sidecar_bags:
        errors.append(
            f"XML declares a{{sv}} property '{prop}' but the sidecar has no matching bag"
        )
    return errors
