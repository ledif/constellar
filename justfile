image := "constellar-builder"
build_dir := "build"

podman_run := "podman run --rm -v " + justfile_directory() + ":/src:Z -w /src " + image

default:
    @just --list

# clang-format
format *files:
    {{podman_run}} bash -c 'clang-format -i $(if [ -n "{{files}}" ]; then echo "{{files}}"; else find src tests -type f \( -name "*.cpp" -o -name "*.h" \); fi)'

# Check clang-format
check-format:
    {{podman_run}} bash -c 'clang-format --dry-run --Werror $(find src tests -type f \( -name "*.cpp" -o -name "*.h" \))'

# Build the container image used for all other recipes.
build-image:
    podman build -t {{image}} -f Containerfile .

# Configure the CMake stuff
configure:
    rm -rf {{build_dir}}
    {{podman_run}} cmake -S . -B {{build_dir}} -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Full CMake build
build:
    #!/bin/bash
    if [ ! -d build ]; then
      just configure
    fi
    {{podman_run}} cmake --build {{build_dir}}

# Run the unit test suite.
test:
    {{podman_run}} ctest --test-dir {{build_dir}} --output-on-failure

# Static-analyze the QML sources (property overrides, unqualified access, type errors).
qmllint:
    {{podman_run}} cmake --build {{build_dir}} --target all_qmllint

# Regenerate ActivityKeys.h from data/io.github.ledif.constellar.spec.yaml
spec-gen:
    {{podman_run}} uv run --project tools/specdoc specdoc --emit keys

# Fail if ActivityKeys.h drifts from the spec sidecar
check-spec:
    {{podman_run}} uv run --project tools/specdoc specdoc --emit keys --check

# Run the daemon against the host session bus and a WoW Logs dir
run-daemon path discord_app_id="1527462779290652672":
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

replay speed="100" slug="midnight-season1-alt-raid":
  #!/bin/bash
  set -euo pipefail
  d=$(mktemp -d)
  daemon_log=$(mktemp)
  trap 'just kill-daemon; rm -rf "$d" "$daemon_log"' EXIT

  just run-daemon "$d" > "$daemon_log" 2>&1 &

  echo "Waiting for constellard to start watching $d..."
  until grep -q "running, watching" "$daemon_log" 2>/dev/null; do
    sleep 0.2
  done

  ./scripts/replay-log.sh "$d" {{speed}} "scripts/fixtures/{{slug}}.tsv"

[private]
kill-daemon:
    podman ps --filter ancestor={{image}} --no-trunc | grep constellard | awk '{print $1}' | xargs -r podman kill

# Run `constellarctl status` against the host session bus inside the container
run-cli *args:
    podman run --rm -it \
        --security-opt label=disable \
        -v {{justfile_directory()}}:/src:Z -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u) \
        -e DBUS_SESSION_BUS_ADDRESS \
        --userns=keep-id \
        {{image}} ./{{build_dir}}/src/cli/constellarctl {{args}}

# Run the daemon+CLI integration test on a private bus inside the container
smoke:
    {{podman_run}} ctest --test-dir {{build_dir}} -L integration -V

# Launch the GUI
run-gui:
    exec podman run --rm -it \
        --security-opt label=disable \
        -v {{justfile_directory()}}:/src:Z -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u) \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        -e DBUS_SESSION_BUS_ADDRESS \
        -e DISPLAY \
        -e WAYLAND_DISPLAY \
        -e XDG_RUNTIME_DIR=/run/user/$(id -u) \
        -e QT_QUICK_CONTROLS_STYLE=org.kde.desktop \
        --userns=keep-id \
        --net=host \
        {{image}} ./{{build_dir}}/src/gui/constellar

clean:
    rm -rf {{build_dir}}

[private]
cmake-format *files:
    {{podman_run}} bash -c 'cmake-format -i $(if [ -n "{{files}}" ]; then echo "{{files}}"; else find . -path ./build -prune -o -type f \( -name CMakeLists.txt -o -name "*.cmake" \) -print; fi)'

[private]
cmake-format-check:
    {{podman_run}} bash -c 'cmake-format --check $(find . -path ./build -prune -o -type f \( -name CMakeLists.txt -o -name "*.cmake" \) -print)'

[private]
cmake-lint:
    {{podman_run}} bash -c 'cmake-lint $(find . -path ./build -prune -o -type f \( -name CMakeLists.txt -o -name "*.cmake" \) -print)'

