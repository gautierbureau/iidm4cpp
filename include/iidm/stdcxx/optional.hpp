#pragma once

// Portable optional type: std::optional (C++17) or boost::optional (C++11/14).
// Use iidm::optional<T> throughout the public API so that the library and its
// consumers always agree on the return type regardless of which C++ standard
// is active — a requirement for ABI consistency when the library is compiled
// via ExternalProject in a C++11 project such as Dynawo.

#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)

#include <optional>

namespace iidm {

template <typename T>
using optional = std::optional<T>;

}  // namespace iidm

#else

#include <boost/optional.hpp>

namespace iidm {

template <typename T>
using optional = boost::optional<T>;

}  // namespace iidm

#endif
