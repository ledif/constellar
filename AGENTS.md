# AGENTS.md

Design docs (RFCs/ADRs/STATUS) live in `agents/`, a separate `jj` repo, git-ignored
here (git owns the code, jj owns `agents/`; see `agents/MEMORY.md`). Read them for
context, but keep code comments self-contained: don't reference doc IDs or paths from
source, since the two histories are decoupled.

## Build

All builds run in a podman container. Assume host has no build tools.

- `just build-image` — (re)build the toolchain image, after Containerfile changes.
- `just setup` — configure CMake (wipes `build/`).
- `just build` — compile.
- `just test` — run `ctest`.
- `just smoke` — daemon + `wowcap status` round trip on a private DBus bus.
- `just format` / `just format-check` — clang-format.

## Conventions

- C++20, DBus interface XML in `data/` is the source of truth
  (`io.github.ledif.wowcapd.Manager`). Client proxy is generated
  (`qdbusxml2cpp`); server adaptor is hand-written. Keep them in sync
  manually when the XML changes.

## Before declaring done

Run `just build && just test && just smoke` (and `just format-check`).
