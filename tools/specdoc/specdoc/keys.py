"""Generation of ActivityKeys.h from the spec sidecar."""

from __future__ import annotations

from .spec import Spec

_MARKER = "// GENERATED from data/io.github.ledif.constellar.spec.yaml"


def _const_line(key: dict) -> str:
    return f'inline constexpr auto {key["const"]} = "{key["key"]}";'


def render_keys_header(spec: Spec) -> str:
    activity = spec.bags["Activity"]
    location = spec.bags["Location"]

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append(_MARKER)
    lines.append("")
    lines.append("// Key vocabulary for the Activity/Zone a{sv} metadata")
    lines.append("namespace constellar::keys")
    lines.append("{")
    lines.append("")

    lines.append("// Activity metadata")
    for key in activity["keys"]:
        lines.append(_const_line(key))
    lines.append("")

    lines.append("// ActivityEnded-only additions")
    for key in activity["ended-additions"]:
        lines.append(_const_line(key))
    lines.append("")

    lines.append("// Location stuff")
    for key in location["keys"]:
        lines.append(_const_line(key))
    lines.append("")

    type_key = next(k for k in activity["keys"] if k["const"] == "kType")
    lines.append(f'// kType ("{type_key["key"]}") values')
    for value in type_key["enum"]:
        const_name = f"kType{value[0].upper()}{value[1:]}"
        lines.append(f'inline constexpr auto {const_name} = "{value}";')
    lines.append("")

    lines.append("}  // namespace constellar::keys")
    lines.append("")

    return "\n".join(lines)
