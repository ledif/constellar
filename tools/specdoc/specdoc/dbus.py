"""Parsing of the D-Bus XML wire shape (dev.ulduar.Constellar.xml).

The sidecar (spec.py) owns prose and the a{sv} key vocabulary; this module
owns interface/method/signal/property *shapes* only, read straight from the
hand-authored introspection XML.
"""

from __future__ import annotations

import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class Arg:
    name: str
    type: str


@dataclass
class Method:
    name: str
    in_args: list[Arg] = field(default_factory=list)
    out_args: list[Arg] = field(default_factory=list)


@dataclass
class Signal:
    name: str
    args: list[Arg] = field(default_factory=list)


@dataclass
class Property:
    name: str
    type: str
    access: str


@dataclass
class Interface:
    name: str
    methods: list[Method]
    signals: list[Signal]
    properties: list[Property]


def _parse_interface_el(iface_el: ET.Element) -> Interface:
    methods = []
    for m in iface_el.findall("method"):
        in_args, out_args = [], []
        for a in m.findall("arg"):
            arg = Arg(name=a.get("name") or "", type=a.get("type"))
            if a.get("direction", "in") == "in":
                in_args.append(arg)
            else:
                out_args.append(arg)
        methods.append(Method(name=m.get("name"), in_args=in_args, out_args=out_args))

    signals = []
    for s in iface_el.findall("signal"):
        args = [Arg(name=a.get("name") or "", type=a.get("type")) for a in s.findall("arg")]
        signals.append(Signal(name=s.get("name"), args=args))

    properties = [
        Property(name=p.get("name"), type=p.get("type"), access=p.get("access"))
        for p in iface_el.findall("property")
    ]

    return Interface(name=iface_el.get("name"), methods=methods, signals=signals, properties=properties)


def parse_interfaces(xml_path: Path) -> dict[str, Interface]:
    """Return every <interface> in the node, keyed by D-Bus name.

    The XML <node> may hold more than one <interface>; nothing here assumes a
    particular count. Order follows the XML document order.
    """
    root = ET.parse(xml_path).getroot()
    interfaces = {}
    for iface_el in root.findall("interface"):
        iface = _parse_interface_el(iface_el)
        interfaces[iface.name] = iface
    return interfaces
