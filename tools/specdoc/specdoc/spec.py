"""Loading and validation of the spec sidecar."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import yaml

from .dbus import parse_interfaces

# Standard freedesktop interfaces are documented in the sidecar's `objects:`
# section (prose only), never redefined member-by-member -- they're not ours
# to specify. Exempt them from the interface-presence and per-member checks.
_STANDARD_INTERFACES = {"org.freedesktop.DBus.ObjectManager"}


@dataclass
class Spec:
    raw: dict
    path: Path

    @property
    def interfaces(self) -> dict:
        """The `interfaces:` map, keyed by D-Bus interface name."""
        return self.raw.get("interfaces", {})

    def interface(self, name: str) -> dict:
        """The sidecar block for one interface (description/bags/methods/...)."""
        return self.interfaces.get(name, {})

    def bags(self, iface_name: str) -> dict:
        """The `a{sv}` bag vocabulary for one interface."""
        return self.interface(iface_name).get("bags", {})


def load_spec(path: Path) -> Spec:
    with path.open("r", encoding="utf-8") as f:
        raw = yaml.safe_load(f)
    return Spec(raw=raw, path=path)


def cross_check(spec: Spec, xml_path: Path) -> list[str]:
    """Return a list of human-readable errors. Empty means no errors.

    Matches sidecar interfaces against XML interfaces by name, then diffs each
    matched pair's bags/methods/signals. Interfaces present on only one side are
    reported so a new interface can't be half-added.
    """
    errors = []
    xml_interfaces = parse_interfaces(xml_path)

    sidecar_names = set(spec.interfaces.keys())
    xml_names = set(xml_interfaces.keys())
    for name in sidecar_names - xml_names:
        errors.append(f"sidecar documents interface '{name}' but the XML has no such interface")
    for name in xml_names - sidecar_names - _STANDARD_INTERFACES:
        errors.append(f"XML declares interface '{name}' but the sidecar has no matching block")

    for name in sorted((sidecar_names & xml_names) - _STANDARD_INTERFACES):
        errors.extend(_cross_check_interface(name, spec.interface(name), xml_interfaces[name]))

    return errors


def _cross_check_interface(name: str, sidecar: dict, interface) -> list[str]:
    errors = []

    xml_props = {p.name for p in interface.properties if p.type == "a{sv}"}
    sidecar_bags = set(sidecar.get("bags", {}).keys())
    for bag in sidecar_bags - xml_props:
        errors.append(
            f"[{name}] sidecar documents bag '{bag}' but the XML has no matching a{{sv}} property"
        )
    for prop in xml_props - sidecar_bags:
        errors.append(
            f"[{name}] XML declares a{{sv}} property '{prop}' but the sidecar has no matching bag"
        )

    xml_scalar_props = {p.name for p in interface.properties if p.type != "a{sv}"}
    sidecar_props = set(sidecar.get("properties", {}).keys())
    for prop in sidecar_props - xml_scalar_props:
        errors.append(
            f"[{name}] sidecar documents property '{prop}' but the XML has no matching property"
        )
    for prop in xml_scalar_props - sidecar_props:
        errors.append(
            f"[{name}] XML declares property '{prop}' but the sidecar has no matching entry"
        )

    xml_methods = {m.name for m in interface.methods}
    sidecar_methods = set(sidecar.get("methods", {}).keys())
    for method in sidecar_methods - xml_methods:
        errors.append(f"[{name}] sidecar documents method '{method}' but the XML has no such method")
    for method in xml_methods - sidecar_methods:
        errors.append(f"[{name}] XML declares method '{method}' but the sidecar has no matching entry")

    xml_signals = {s.name for s in interface.signals}
    sidecar_signals = set(sidecar.get("signals", {}).keys())
    for signal in sidecar_signals - xml_signals:
        errors.append(f"[{name}] sidecar documents signal '{signal}' but the XML has no such signal")
    for signal in xml_signals - sidecar_signals:
        errors.append(f"[{name}] XML declares signal '{signal}' but the sidecar has no matching entry")

    return errors
