image := "constellar-builder"
build_dir := "build"

podman_run := "podman run --rm -v " + justfile_directory() + ":/src:Z -w /src " + image

default:
    @just --list

# Reformat C++ sources with clang-format. With no args, formats everything
# under src/; pass paths to format only those (used by the lefthook
# pre-commit hook, which passes the staged files).
format *files:
    {{podman_run}} bash -c 'clang-format -i $(if [ -n "{{files}}" ]; then echo "{{files}}"; else find src tests -type f \( -name "*.cpp" -o -name "*.h" \); fi)'

# Check formatting without modifying files; exits non-zero on drift (CI use).
format-check:
    {{podman_run}} bash -c 'clang-format --dry-run --Werror $(find src tests -type f \( -name "*.cpp" -o -name "*.h" \))'

# Build the container image used for all other recipes.
build-image:
    podman build -t {{image}} -f container/Containerfile .

# Configure the CMake build directory (idempotent; re-run after adding files).
setup:
    rm -rf {{build_dir}}
    {{podman_run}} cmake -S . -B {{build_dir}} -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Compile.
build:
    {{podman_run}} cmake --build {{build_dir}}

# Run the unit test suite.
test:
    {{podman_run}} ctest --test-dir {{build_dir}} --output-on-failure

# Drop into an interactive shell in the build container.
shell:
    podman run --rm -it -v {{justfile_directory()}}:/src:Z -w /src {{image}} bash

# Run the daemon against the host session bus, inside the container.
# Requires a log directory; see run-daemon-live for pointing it at a real
# WoW Logs directory mounted read-only.
run-daemon log_dir:
    exec podman run --rm -it \
        --security-opt label=disable \
        -v {{justfile_directory()}}:/src:Z -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u) \
        -e DBUS_SESSION_BUS_ADDRESS \
        -e XDG_RUNTIME_DIR=/run/user/$(id -u) \
        --userns=keep-id \
        {{image}} ./{{build_dir}}/src/daemon/constellard --log-dir {{log_dir}}

# Run the daemon against the host session bus and a real WoW Logs directory
# (read-only mount, SELinux relabeling disabled like log-tail). This plus
# `just run-gui` in a second terminal is the live detection MVP. Pass
# discord_app_id to enable Rich Presence (RFC-002); left empty, presence
# stays off (main.cpp no-ops without an app ID). /run/user is shared
# rootless-netns state across concurrent podman processes and can't be
# exclusively relabeled (:Z) without racing other containers — same fix as
# run-gui. `exec`'d so podman replaces the recipe's shell as PID 1 of the
# foreground process group -- otherwise Ctrl-C's SIGINT can get eaten by
# the wrapper shell instead of reaching the container.
run-daemon-live path discord_app_id="1527462779290652672":
    exec podman run --rm -it \
        --security-opt label=disable \
        -v {{justfile_directory()}}:/src:Z -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u) \
        -v "{{path}}":/wow-logs:ro \
        -e DBUS_SESSION_BUS_ADDRESS \
        -e XDG_RUNTIME_DIR=/run/user/$(id -u) \
        -e CONSTELLAR_DISCORD_APP_ID={{discord_app_id}} \
        --userns=keep-id \
        {{image}} ./{{build_dir}}/src/daemon/constellard --log-dir /wow-logs

# Kill a daemon started by run-daemon/run-daemon-live from another terminal,
# for when Ctrl-C in that terminal doesn't reach the container (podman/TTY
# signal-forwarding quirk, seen even with the run-daemon-live `exec` fix).
# No-ops quietly if nothing is running.
kill-daemon:
    podman ps --filter ancestor={{image}} --no-trunc | grep constellard | awk '{print $1}' | xargs -r podman kill

# Run `constellar status` against the host session bus, inside the container.
run-cli *args:
    podman run --rm -it \
        --security-opt label=disable \
        -v {{justfile_directory()}}:/src:Z -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u) \
        -e DBUS_SESSION_BUS_ADDRESS \
        --userns=keep-id \
        {{image}} ./{{build_dir}}/src/cli/constellar {{args}}

# Run a self-contained daemon+CLI smoke test on a private dbus-run-session
# bus inside the container. No host session bus required — good for CI.
smoke:
    {{podman_run}} dbus-run-session -- bash -c ' \
        ./{{build_dir}}/src/daemon/constellard --log-dir /tmp & \
        pid=$!; \
        sleep 1; \
        ./{{build_dir}}/src/cli/constellar status; \
        status=$?; \
        kill $pid; \
        exit $status \
    '

# Run the GUI against the host session+display, inside the container.
# /tmp/.X11-unix is a shared system socket dir owned outside our user/SELinux
# domain, so it can't be exclusively relabeled (:Z) the way /src and
# /run/user can — disable SELinux confinement for this container instead
# (same fix as log-tail's real-game-install mount). `exec`'d for clean
# Ctrl-C (see run-daemon-live).
run-gui:
    exec podman run --rm -it \
        --security-opt label=disable \
        -v {{justfile_directory()}}:/src -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u) \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        -e DBUS_SESSION_BUS_ADDRESS \
        -e DISPLAY \
        -e WAYLAND_DISPLAY \
        -e XDG_RUNTIME_DIR=/run/user/$(id -u) \
        --userns=keep-id \
        --net=host \
        {{image}} ./{{build_dir}}/src/gui/constellar-gui

# Tail a real WoW Logs directory (read-only) and print every parsed LogLine,
# to sanity-check LogLine/LogWatcher against a live client.
log-tail path idle="60000":
    podman run --rm -it \
        --security-opt label=disable \
        -v {{justfile_directory()}}:/src -w /src \
        -v "{{path}}":/wow-logs:ro \
        {{image}} ./{{build_dir}}/tools/logtail/logtail /wow-logs {{idle}}

clean:
    rm -rf {{build_dir}}
