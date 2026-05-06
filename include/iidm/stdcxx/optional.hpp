#pragma once

// Portable optional type: std::optional (C++17) or boost::optional (C++11/14).
//
// IIDM_BRIDGE_USE_STD_OPTIONAL / IIDM_BRIDGE_USE_BOOST_OPTIONAL are exported
// as PUBLIC compile definitions by the iidmbridge CMake target and encode which
// optional type was compiled into the library.  They override the C++-standard
// sniff below so that consumers using a *different* standard than the library
// (e.g. Dynawo C++14 consuming a C++17-built iidmbridge) still resolve
// iidm::optional<T> to the same type — avoiding MSVC LNK2019 / Itanium ODR
// violations caused by std::optional vs boost::optional ABI differences.

#if defined(IIDM_BRIDGE_USE_STD_OPTIONAL)

#include <optional>
namespace iidm {
template <typename T> using optional = std::optional<T>;
}

#elif defined(IIDM_BRIDGE_USE_BOOST_OPTIONAL)

#include <boost/optional.hpp>
namespace iidm {
template <typename T> using optional = boost::optional<T>;
}

#else

// No explicit marker: standalone / non-CMake build.
// Detect from the active C++ standard.
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#include <optional>
namespace iidm {
template <typename T> using optional = std::optional<T>;
}
#else
#include <boost/optional.hpp>
namespace iidm {
template <typename T> using optional = boost::optional<T>;
}
#endif

#endif
