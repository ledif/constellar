# wowcapd

World of Warcraft recording service for Linux. A background daemon watches the
WoW combat log, detects encounters and records them with libobs.

Built for Wayland/PipeWire and Proton. Packaged as a Flatpak.

Inspired by the Windows-only [wow-recorder](https://github.com/aza547/wow-recorder).

## Architecture

The `wowcapd` daemon publishes on the DBus session bus (`io.github.ledif.wowcapd`) and automatically records encounters.

Other components:
- A small Qt GUI for configuration. Drives the directory picker and Wayland capture portals.
- A simple `wowcap` CLI to interact with the daemon
- Future: Rich Discord Presence

## Building

Everything builds inside a podman container

```sh
just build-image   # once, and after Containerfile changes
just setup         # configure CMake
just build         # compile
just test          # run the test suite
```

## Status

Early development. wowcapd is not yet feature-complete or packaged for end users.

## License

MIT
