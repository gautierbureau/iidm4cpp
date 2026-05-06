#pragma once

#include <cstdint>

// iidm-bridge-cpp requires C++17.  On MSVC this means /std:c++17; on GCC/Clang
// -std=c++17.  If you reach this error the consuming project is not enabling the
// required standard.  When using CMake, link against the iidmbridge target and
// the requirement propagates automatically via target_compile_features.
#if __cplusplus < 201703L && !(defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
static_assert(false, "iidm-bridge-cpp requires C++17 or later (/std:c++17 on MSVC, -std=c++17 on GCC/Clang).");
#endif

namespace iidm {

// An ObjectHandle is an opaque integer that uniquely identifies a Java
// object within a backend session.  It is valid for the lifetime of the
// Network that produced it.
using ObjectHandle = std::int64_t;
constexpr ObjectHandle INVALID_HANDLE = 0;

} // namespace iidm
