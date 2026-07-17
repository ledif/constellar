image := "constellar-builder"
build_dir := "build"

podman_run := "podman run --rm -v " + justfile_directory() + ":/src:Z -w /src " + image

default:
    @just --list

# Format either a list of files, or everything in src/ and tests/
format *files:
    {{podman_run}} bash -c 'clang-format -i $(if [ -n "{{files}}" ]; then echo "{{files}}"; else find src tests -type f \( -name "*.cpp" -o -name "*.h" \); fi)'

# Check clang-format
format-check:
    {{podman_run}} bash -c 'clang-format --dry-run --Werror $(find src tests -type f \( -name "*.cpp" -o -name "*.h" \))'

# Build the container image used for all other recipes.
build-image:
    podman build -t {{image}} -f Containerfile .

# Configure the CMake stuff
configure:
    rm -rf {{build_dir}}
    {{podman_run}} cmake -S . -B {{build_dir}} -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo

build:
    #!/bin/bash
    if [ ! -d build ]; then
      just configure
    fi
    {{podman_run}} cmake --build {{build_dir}}

# Run the unit test suite.
test:
    {{podman_run}} ctest --test-dir {{build_dir}} --output-on-failure

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

kill-daemon:
    podman ps --filter ancestor={{image}} --no-trunc | grep constellard | awk '{print $1}' | xargs -r podman kill

# Run `constellar status` against the host session bus inside the container
run-cli *args:
    podman run --rm -it \
        --security-opt label=disable \
        -v {{justfile_directory()}}:/src:Z -w /src \
        -v /run/user/$(id -u):/run/user/$(id -u) \
        -e DBUS_SESSION_BUS_ADDRESS \
        --userns=keep-id \
        {{image}} ./{{build_dir}}/src/cli/constellar {{args}}

# Run daemon+CLI smoke test on a private bus inside the container
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

# Launch the GUI
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

clean:
    rm -rf {{build_dir}}
