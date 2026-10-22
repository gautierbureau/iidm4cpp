# Downloads GraalVM CE for the host into the build tree, and sets NATIVE_IMAGE_EXECUTABLE to its
# native-image. CMakeLists.txt includes it when no native-image is installed and
# IIDM_BRIDGE_DOWNLOAD_GRAALVM is ON.
#
# GraalVM CE 21.0.2 is the last Community release on JDK 21, the one the graalvm CI job installs.
# The checksums are those of the .sha256 files published with the release.

include(FetchContent)

set(_GRAALVM_VERSION 21.0.2)
set(_GRAALVM_SHA256_linux-x64     b048069aaa3a99b84f5b957b162cc181a32a4330cbc35402766363c5be76ae48)
set(_GRAALVM_SHA256_linux-aarch64 a34be691ce68f0acf4655c7c6c63a9a49ed276a11859d7224fd94fc2f657cd7a)
set(_GRAALVM_SHA256_macos-x64     7a8aa93fa45d1721908477abf4732a32637d420ffcb66ada9fb6456440b0d9e1)
set(_GRAALVM_SHA256_macos-aarch64 515e3a93acc7e1938daba83eda4272e5495fd302d7cdd99ec7ebf408ed505ab7)
set(_GRAALVM_SHA256_windows-x64   e17b7bead097bf372a5c75df17815b0a2f30b777a019d25eff7706b21421f7fa)

set(_GRAALVM_OS "")
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(_GRAALVM_OS linux)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(_GRAALVM_OS macos)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(_GRAALVM_OS windows)
endif()
set(_GRAALVM_ARCH "")
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
    set(_GRAALVM_ARCH x64)
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
    set(_GRAALVM_ARCH aarch64)
endif()
set(_GRAALVM_PLATFORM "${_GRAALVM_OS}-${_GRAALVM_ARCH}")
if(NOT DEFINED _GRAALVM_SHA256_${_GRAALVM_PLATFORM})
    message(FATAL_ERROR
        "native-image not found, and GraalVM CE ${_GRAALVM_VERSION} has no download for this host "
        "(${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR}): install GraalVM CE for JDK 21, "
        "or turn IIDM_BRIDGE_BUILD_NATIVE off.")
endif()

if(_GRAALVM_OS STREQUAL "windows")
    set(_GRAALVM_ARCHIVE zip)
else()
    set(_GRAALVM_ARCHIVE tar.gz)
endif()
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW) # the extracted files get the time of the extraction
endif()
FetchContent_Declare(graalvm
    URL "https://github.com/graalvm/graalvm-ce-builds/releases/download/jdk-${_GRAALVM_VERSION}/graalvm-community-jdk-${_GRAALVM_VERSION}_${_GRAALVM_PLATFORM}_bin.${_GRAALVM_ARCHIVE}"
    URL_HASH SHA256=${_GRAALVM_SHA256_${_GRAALVM_PLATFORM}}
)
message(STATUS "native-image not found: downloading GraalVM CE ${_GRAALVM_VERSION} for "
               "${_GRAALVM_PLATFORM} (about 290 MB) into ${FETCHCONTENT_BASE_DIR}")
FetchContent_MakeAvailable(graalvm)

set(_GRAALVM_HOME "${graalvm_SOURCE_DIR}")
if(_GRAALVM_OS STREQUAL "macos")
    set(_GRAALVM_HOME "${graalvm_SOURCE_DIR}/Contents/Home")
endif()
find_program(NATIVE_IMAGE_EXECUTABLE NAMES native-image native-image.cmd
             PATHS "${_GRAALVM_HOME}/bin" NO_DEFAULT_PATH)
