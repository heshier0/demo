host_build {
    QT_ARCH = x86_64
    QT_BUILDABI = x86_64-little_endian-lp64
    QT_TARGET_ARCH = arm
    QT_TARGET_BUILDABI = arm-little_endian-ilp32-eabi
} else {
    QT_ARCH = arm
    QT_BUILDABI = arm-little_endian-ilp32-eabi
}
QT.global.enabled_features = cross_compile shared c++11 c++14 c++1z concurrent
QT.global.disabled_features = framework rpath appstore-compliant debug_and_release simulator_and_device build_all force_asserts pkg-config separate_debug_info static
CONFIG += cross_compile shared release
QT_CONFIG += shared release c++11 c++14 c++1z concurrent no-pkg-config reduce_exports stl
QT_VERSION = 5.9.8
QT_MAJOR_VERSION = 5
QT_MINOR_VERSION = 9
QT_PATCH_VERSION = 8
QT_GCC_MAJOR_VERSION = 6
QT_GCC_MINOR_VERSION = 3
QT_GCC_PATCH_VERSION = 0
QT_EDITION = OpenSource
