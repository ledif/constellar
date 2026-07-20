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
        qt6-qtdeclarative-devel \
        extra-cmake-modules \
        kf6-kirigami-devel \
        kf6-kirigami-addons-devel \
        kf6-ki18n-devel \
        kf6-kcoreaddons-devel \
        kf6-qqc2-desktop-style \
        kf6-kiconthemes-devel \
        yaml-cpp-devel \
        sqlite-devel \
        dbus-devel \
        dbus-daemon \
        dbus-tools \
        gdb \
        valgrind \
        libasan \
        libubsan \
        python3.12 \
    && dnf clean all

RUN python3.12 -m ensurepip --upgrade \
    && python3.12 -m pip install --no-cache-dir cmakelang pyyaml

WORKDIR /src
