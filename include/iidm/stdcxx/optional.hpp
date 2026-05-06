#pragma once

// Portable optional type: std::optional (C++17) or boost::optional (C++11/14).
//
// IIDM_BRIDGE_USE_STD_OPTIONAL / IIDM_BRIDGE_USE_BOOST_OPTIONAL are exported
// as PUBLIC compile definitions by the iidmbridge CMake target.  They encode
// which optional type was compiled into the library so that every consumer uses
// the identical type — a hard requirement for ABI compatibility.
//
// Without this guard the linker mismatch manifests differently per platform:
//   MSVC  – LNK2019 (mangled names differ: std@@... vs boost@@...)
//   Linux – silent link, then a segfault at runtime because std::optional and
//           boost::optional store the bool flag and the value at opposite ends
//           of their 16-byte layout; reading the wrong offset yields garbage.
//
// When a mismatch is detected a clear #error is raised, directing the user to
// rebuild iidmbridge with the matching -DCMAKE_CXX_STANDARD.

#if defined(IIDM_BRIDGE_USE_STD_OPTIONAL)

// Library compiled with C++17 → std::optional ABI.
// Verify the consumer is also in C++17 mode; if not, the mangled symbols and
// the in-memory layout will disagree, causing linker errors or crashes.
#  if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
#    error "iidmbridge was compiled with C++17 (std::optional ABI) but your " \
           "consumer is in C++14 mode. Rebuild iidmbridge with " \
           "-DCMAKE_CXX_STANDARD=14 (and Boost available) so the library " \
           "exports boost::optional symbols that match your consumer."
#  endif
#  include <optional>
namespace iidm {
template <typename T> using optional = std::optional<T>;
}

#elif defined(IIDM_BRIDGE_USE_BOOST_OPTIONAL)

// Library compiled with C++14/11 → boost::optional ABI.
// All consumers (any C++ standard) use boost::optional, matching the symbols.
#  include <boost/optional.hpp>
namespace iidm {
template <typename T> using optional = boost::optional<T>;
}

#else

// No marker: standalone / non-CMake build.  Detect from the active standard.
#  if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#    include <optional>
namespace iidm {
template <typename T> using optional = std::optional<T>;
}
#  else
#    include <boost/optional.hpp>
namespace iidm {
template <typename T> using optional = boost::optional<T>;
}
#  endif

#endif
