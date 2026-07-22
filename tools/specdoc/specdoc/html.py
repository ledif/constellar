"""Generation of the MPRIS-style HTML reference.

Reads *shapes* from the XML (via dbus.parse_interfaces) and *prose + key
vocabulary* from the sidecar (spec.Spec), and renders one landing page
(doc/spec/index.html) plus one page per interface
(doc/spec/<ShortName>_Interface.html).
"""

from __future__ import annotations

from pathlib import Path

from jinja2 import Environment, FileSystemLoader, select_autoescape

from .dbus import Interface
from .spec import Spec


def short_name(interface_name: str) -> str:
    """Trailing dot-component of a D-Bus interface name (the display name)."""
    return interface_name.rsplit(".", 1)[-1]


def interface_filename(interface_name: str) -> str:
    """Per-interface page filename, e.g. Observer_Interface.html."""
    return f"{short_name(interface_name)}_Interface.html"


def _stability(prose_map: dict, name: str) -> str:
    return prose_map.get(name, {}).get("stability", "stable")


def _description(prose_map: dict, name: str) -> str:
    return prose_map.get(name, {}).get("description", "")


def _make_env(templates_dir: Path) -> Environment:
    return Environment(
        loader=FileSystemLoader(str(templates_dir)),
        autoescape=select_autoescape(["html"]),
    )


def _interface_context(spec: Spec, interface: Interface) -> dict:
    """Build the render context for one interface's page, drawing shapes from
    the parsed XML `interface` and prose/keys from its sidecar block."""
    sidecar = spec.interface(interface.name)
    method_prose = sidecar.get("methods", {})
    signal_prose = sidecar.get("signals", {})
    bags_spec = sidecar.get("bags", {})

    methods = [
        {
            "name": m.name,
            "in_args": m.in_args,
            "out_args": m.out_args,
            "description": _description(method_prose, m.name),
            "stability": _stability(method_prose, m.name),
        }
        for m in interface.methods
    ]
    signals = [
        {
            "name": s.name,
            "args": s.args,
            "description": _description(signal_prose, s.name),
            "stability": _stability(signal_prose, s.name),
        }
        for s in interface.signals
    ]

    bag_description = {name: bag["description"] for name, bag in bags_spec.items()}
    properties = [
        {
            "name": p.name,
            "type": p.type,
            "access": p.access,
            "description": bag_description.get(p.name, ""),
        }
        for p in interface.properties
    ]

    bags = []
    enums = []
    for name, bag in bags_spec.items():
        keys = bag.get("keys", [])
        ended = bag.get("ended-additions", [])
        bags.append(
            {
                "name": name,
                "slug": name.lower(),
                "description": bag["description"],
                "entries": keys,
                "ended_additions": ended,
                "has_applies_to": any(k.get("applies-to") for k in keys + ended),
            }
        )
        for key in keys + ended:
            if key.get("enum"):
                enums.append({"const": key["const"], "slug": key["const"].lower(), "members": key["enum"]})

    return {
        "name": interface.name,
        "short_name": short_name(interface.name),
        "description": sidecar.get("description", ""),
        "methods": methods,
        "signals": signals,
        "properties": properties,
        "bags": bags,
        "enums": enums,
        "errors": sidecar.get("errors", []),
    }


def render_index(
    spec: Spec,
    interfaces: dict[str, Interface],
    templates_dir: Path,
    license_text: str,
) -> str:
    """Render the landing page: title, version, legal, About, interface list.

    The legal section embeds the full text of the project LICENSE (which carries
    its own copyright line), rather than a summary kept in the sidecar."""
    listing = [
        {
            "name": iface.name,
            "short_name": short_name(iface.name),
            "href": interface_filename(iface.name),
            "description": spec.interface(iface.name).get("description", ""),
        }
        for iface in interfaces.values()
    ]
    template = _make_env(templates_dir).get_template("index.html")
    return template.render(
        spec_version=spec.raw.get("spec-version"),
        license=license_text,
        about=spec.raw.get("about", ""),
        interfaces=listing,
    )


def render_interface(spec: Spec, interface: Interface, templates_dir: Path) -> str:
    """Render one interface's reference page."""
    template = _make_env(templates_dir).get_template("interface.html")
    return template.render(
        spec_version=spec.raw.get("spec-version"),
        interface=_interface_context(spec, interface),
    )


def check_index_coverage(html: str, interfaces: dict[str, Interface]) -> list[str]:
    """Every interface must appear (and link out) on the landing page."""
    errors = []
    for iface in interfaces.values():
        if iface.name not in html:
            errors.append(f"interface '{iface.name}' is missing from the landing page")
        if interface_filename(iface.name) not in html:
            errors.append(f"landing page has no link to '{interface_filename(iface.name)}'")
    return errors


def check_interface_coverage(html: str, spec: Spec, interface: Interface) -> list[str]:
    """Literal substring check: every documented key/member of this interface
    must appear in its own rendered page. Catches a template that silently drops
    a section, independent of how the render context was built."""
    errors = []
    sidecar = spec.interface(interface.name)

    for bag_name, bag in sidecar.get("bags", {}).items():
        for key in bag.get("keys", []) + bag.get("ended-additions", []):
            if key["const"] not in html:
                errors.append(
                    f"[{interface.name}] key '{key['const']}' (bag '{bag_name}') has no rendered section"
                )

    for method in interface.methods:
        if method.name not in html:
            errors.append(f"[{interface.name}] method '{method.name}' has no rendered section")
    for signal in interface.signals:
        if signal.name not in html:
            errors.append(f"[{interface.name}] signal '{signal.name}' has no rendered section")
    for prop in interface.properties:
        if prop.name not in html:
            errors.append(f"[{interface.name}] property '{prop.name}' has no rendered section")

    for err in sidecar.get("errors", []):
        if err["code"] not in html:
            errors.append(f"[{interface.name}] error code '{err['code']}' has no rendered section")

    return errors
