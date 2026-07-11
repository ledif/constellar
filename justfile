image := "wowcapd-builder"
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
run-daemon:
    podman run --rm -it \
        -v {{justfile_directory()}}:/src:Z -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u):Z \
        -e DBUS_SESSION_BUS_ADDRESS \
        --userns=keep-id \
        {{image}} ./{{build_dir}}/src/daemon/wowcapd

# Run `wowcap status` against the host session bus, inside the container.
run-cli *args:
    podman run --rm -it \
        -v {{justfile_directory()}}:/src:Z -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u):Z \
        -e DBUS_SESSION_BUS_ADDRESS \
        --userns=keep-id \
        {{image}} ./{{build_dir}}/src/cli/wowcap {{args}}

# Run a self-contained daemon+CLI smoke test on a private dbus-run-session
# bus inside the container. No host session bus required — good for CI.
smoke:
    {{podman_run}} dbus-run-session -- bash -c ' \
        ./{{build_dir}}/src/daemon/wowcapd & \
        pid=$!; \
        sleep 1; \
        ./{{build_dir}}/src/cli/wowcap status; \
        status=$?; \
        kill $pid; \
        exit $status \
    '

# Run the GUI against the host session+display, inside the container.
run-gui:
    podman run --rm -it \
        -v {{justfile_directory()}}:/src:Z -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u):Z \
        -v /tmp/.X11-unix:/tmp/.X11-unix:Z \
        -e DBUS_SESSION_BUS_ADDRESS \
        -e DISPLAY \
        -e WAYLAND_DISPLAY \
        --userns=keep-id \
        --net=host \
        {{image}} ./{{build_dir}}/src/gui/wowcapd-gui

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
