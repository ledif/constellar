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


def _split_path(path: str) -> tuple[str, str | None]:
    """Split an object path into its literal prefix and a trailing
    `<placeholder>` segment, if any, so templates can wrap the latter in
    `<var>` instead of rendering it as though it were a literal path."""
    prefix, _, last = path.rpartition("/")
    if last.startswith("<") and last.endswith(">"):
        return f"{prefix}/", last
    return path, None


def _escape_for_coverage(text: str) -> str:
    """Mirror Jinja's autoescaping of `<`/`>` so coverage checks can look for
    a placeholder's rendered form as a literal substring."""
    return text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def _exposed_at_row(entry: dict) -> dict:
    """Shape one object-tree (or root) entry for an interface page's
    *Exposed at* list: just the path, split for `<var>` wrapping."""
    prefix, placeholder = _split_path(entry["path"])
    return {"path": entry["path"], "prefix": prefix, "placeholder": placeholder}


def _exposed_at(spec: Spec, interface_name: str, object_tree: list[dict]) -> list[dict]:
    """Object-tree (and root) entries whose `interfaces` include this one,
    shaped for the interface page's *Exposed at* list."""
    entries = [entry for entry in object_tree if interface_name in entry["interfaces"]]
    if interface_name in spec.root_entry["interfaces"]:
        entries = entries + [spec.root_entry]
    return [_exposed_at_row(entry) for entry in entries]


def _tree_split(path: str, root: str) -> tuple[str, str, str | None]:
    """Split a path into `(gray, leaf, placeholder)`: `gray` is the shared
    `objects.root` segment (de-emphasized in the template), `leaf` is the
    distinguishing tail, and `placeholder` is a trailing `<id>`-style segment
    to wrap in `<var>`."""
    root_prefix = root if root.endswith("/") else f"{root}/"
    if root and path.startswith(root_prefix):
        gray, rest = root_prefix, path[len(root_prefix) :]
    else:
        gray, rest = "", path
    body, _, last = rest.rpartition("/")
    if last.startswith("<") and last.endswith(">"):
        return gray, f"{body}/" if body else "", last
    return gray, rest, None


def _index_object_row(entry: dict, root: str) -> dict:
    """Shape one object-tree (or root) entry for the landing page's Objects
    list: tree-split path, and interfaces resolved to a primary type plus any
    additional ones it also implements."""
    gray, leaf, placeholder = _tree_split(entry["path"], root)
    ifaces = [
        {"name": name, "short_name": short_name(name), "href": interface_filename(name)}
        for name in entry["interfaces"]
    ]
    return {
        "path": entry["path"],
        "gray": gray,
        "leaf": leaf,
        "placeholder": placeholder,
        "primary": ifaces[0] if ifaces else None,
        "also": ifaces[1:],
        "stability": entry["stability"],
        "status": entry["status"],
        "description": entry["description"],
    }


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
    prop_spec = sidecar.get("properties", {})
    properties = [
        {
            "name": p.name,
            "type": p.type,
            "access": p.access,
            "description": bag_description.get(p.name) or _description(prop_spec, p.name),
            "stability": _stability(prop_spec, p.name) if p.name in prop_spec else "stable",
        }
        for p in interface.properties
    ]

    bags = []
    enums = []
    for prop_name, prop in prop_spec.items():
        if prop.get("enum"):
            enums.append({"const": prop_name, "slug": prop_name.lower(), "members": prop["enum"]})
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

    description = sidecar.get("description", "")
    if not description and interface.name in spec.root_entry["interfaces"]:
        description = spec.root_entry["description"]

    return {
        "name": interface.name,
        "short_name": short_name(interface.name),
        "description": description,
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
    root = spec.objects.get("root", "")
    root_row = dict(spec.root_entry)
    root_row["description"] = ""  # the implementation prose lives on the ObjectManager page

    template = _make_env(templates_dir).get_template("index.html")
    return template.render(
        spec_version=spec.raw.get("spec-version"),
        license=license_text,
        about=spec.raw.get("about", ""),
        interfaces=listing,
        object_tree=[_index_object_row(entry, root) for entry in spec.object_tree]
        + [_index_object_row(root_row, root)],
    )


def render_interface(
    spec: Spec, interface: Interface, templates_dir: Path, object_tree: list[dict]
) -> str:
    """Render one interface's reference page."""
    template = _make_env(templates_dir).get_template("interface.html")
    context = _interface_context(spec, interface)
    context["exposed_at"] = _exposed_at(spec, interface.name, object_tree)
    return template.render(
        spec_version=spec.raw.get("spec-version"),
        interface=context,
    )


def check_index_coverage(html: str, interfaces: dict[str, Interface], spec: Spec) -> list[str]:
    """Every interface must appear (and link out) on the landing page, and
    every object-tree path must be rendered in the Object Tree section."""
    errors = []
    for iface in interfaces.values():
        if iface.name not in html:
            errors.append(f"interface '{iface.name}' is missing from the landing page")
        if interface_filename(iface.name) not in html:
            errors.append(f"landing page has no link to '{interface_filename(iface.name)}'")

    root = spec.objects.get("root", "")
    for entry in list(spec.object_tree) + [spec.root_entry]:
        gray, leaf, placeholder = _tree_split(entry["path"], root)
        if (
            (gray and gray not in html)
            or (leaf and leaf not in html)
            or (placeholder and _escape_for_coverage(placeholder) not in html)
        ):
            errors.append(f"object path '{entry['path']}' is missing from the landing page")

    return errors


def check_interface_coverage(
    html: str, spec: Spec, interface: Interface, object_tree: list[dict]
) -> list[str]:
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

    for entry in _exposed_at(spec, interface.name, object_tree):
        if entry["prefix"] not in html or (
            entry["placeholder"] and _escape_for_coverage(entry["placeholder"]) not in html
        ):
            errors.append(
                f"[{interface.name}] object path '{entry['path']}' is missing from Exposed at"
            )

    return errors
