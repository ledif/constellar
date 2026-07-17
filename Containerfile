FROM fedora:rawhide

RUN dnf install -y --setopt=install_weak_deps=False \
        gcc-c++ \
        cmake \
        ninja-build \
        pkgconf-pkg-config \
        git \
        clang-tools-extra \
        qt6-qtbase-devel \
        qt6-qttools-devel \
        yaml-cpp-devel \
        sqlite-devel \
        dbus-devel \
        dbus-daemon \
        dbus-tools \
        python3.12 \
    && dnf clean all

RUN python3.12 -m ensurepip --upgrade \
    && python3.12 -m pip install --no-cache-dir cmakelang pyyaml

WORKDIR /src
