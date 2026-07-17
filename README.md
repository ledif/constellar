# Constellar

A D-Bus daemon and set of clients for World of Warcraft. The `constallard` daemon publishes game state (idling in Silvermoon, bricking a +12 key) for downstream clients to consume. A Discord presence client and a libobs-based video recording client are provided by default.

Built for Wayland and packaged as a Flatpak.

## Architecture

The D-Bus interface aspires to be something like [MPRIS](https://wiki.archlinux.org/title/MPRIS) for World of Warcraft. That is, any D-Bus client should be able to query the state of the player in the game world.

Other components:
- A small Qt GUI for configuration. Drives the directory picker and Wayland capture portals.
- A simple `constellar` CLI to interact with the daemon
- A Discord presence client

## Development and Contributing

Everything builds inside a podman container

```sh
just build-image   # once, and after Containerfile changes
just setup         # configure CMake
just build         # compile
just test          # run the test suite
```

All code / assets in the `main` branch must be written by a human or manually reviewed by a human. Agent-written code generally lives in `slop/` branches.

## Status

Early development. Constellar is not yet feature-complete or packaged for end users.

## AI Disclosure

Claude was used to write large portions of this codebase. Everything in the `main` branch is manually reviewed and tested by a human (an actual C++ dev) on a best-effort basis.

## License

MIT
