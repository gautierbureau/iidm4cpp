# FindGraalVM.cmake
#
# Locates the GraalVM Native Image SDK headers (graal_isolate.h, etc.).
#
# Inputs:
#   GRAALVM_HOME  — path to a GraalVM installation (e.g. /usr/lib/jvm/graalvm)
#
# Outputs:
#   GraalVM_FOUND
#   GRAALVM_INCLUDE_DIRS

if(DEFINED ENV{GRAALVM_HOME})
    set(GRAALVM_HOME "$ENV{GRAALVM_HOME}" CACHE PATH "GraalVM installation root")
endif()

find_path(GRAALVM_INCLUDE_DIRS
    NAMES graal_isolate.h
    HINTS
        ${GRAALVM_HOME}/include
        ${GRAALVM_HOME}/lib/svm/clibraries/linux-amd64/include
        /usr/local/include
        /usr/include
    DOC "GraalVM Native Image header directory"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GraalVM
    REQUIRED_VARS GRAALVM_INCLUDE_DIRS
)

mark_as_advanced(GRAALVM_INCLUDE_DIRS)
