# Constellar

A D-Bus daemon and set of clients for World of Warcraft. The `constallard` daemon publishes game state for downstream clients to consume. A Discord presence client (implemented) and a libobs-based video recording client (future) are in scope.

## Build

All builds run in a podman container. Assume host has no build tools besides uv.

- `just build-image`: (re)build the toolchain image, after Containerfile changes.
- `just configure`: configure CMake (wipes `build/`).
- `just build`: compile.
- `just test`: run `ctest`.
- `just smoke`: daemon + `constellarctl` round trip on a private DBus bus.
- `just format` / `just check-format`: clang-format.
- `just cmake-lint`: cmake-lint style/anti-pattern checks.
- `just qmllint`: static analysis of `src/gui/qml/*.qml`

## Conventions

- C++26 and Qt6.
- DBus interface XML in `data/` is the source of truth for the wire shape (`dev.ulduar.Constellar.Observer`). Client proxy is generated (`qdbusxml2cpp`); server adaptor is hand-written. Keep them in sync manually when the XML changes.
- The `a{sv}` key vocabulary's source of truth is `data/dev.ulduar.Constellar.spec.yaml` and `src/common/ActivityKeys.h` is generated from it (`just spec-gen`, checked by `just check-spec`).
- CMake is hand-formatted, cmake-format is for reference

### Qt Conventions

- String literals: use `u"..."_s` (`Qt::StringLiterals`) not `QStringLiteral`.

## Agent Conventions 
- Design docs and the agent knowledge-base lives in `agents/`, a separate git-ignored directory.
- Read top-level Markdown files in this directory first when looking for information.
- Everything that lands in `main` needs to be self-contained so don't reference document
material (IDs, RFCs, etc) in code.
- Before declaring done, run `just build && just test` (and `just format`).
