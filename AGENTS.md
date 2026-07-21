# AGENTS.md

Design docs and the agent knowledge-base lives in `agents/`, a separate git-ignored directory.
Read top-level Markdown files in this directory first when looking for information.
Everything that lands in `main` needs to be self-contained so don't reference document
material (IDs, RFCs, etc) in code.

## Build

All builds run in a podman container. Assume host has no build tools.

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
- DBus interface XML in `data/` is the source of truth (`io.github.ledif.constellar.Observer`). Client proxy is generated (`qdbusxml2cpp`); server adaptor is hand-written. Keep them in sync manually when the XML changes.
- CMake is hand-formatted, cmake-format is for reference

### Qt Conventions

- String literals: use `u"..."_s` (`Qt::StringLiterals`) not `QStringLiteral`.

## Before declaring done

Run `just build && just test` (and `just format`).
