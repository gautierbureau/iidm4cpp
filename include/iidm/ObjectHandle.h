#pragma once

#include <cstdint>

namespace iidm {

// An ObjectHandle is an opaque integer that uniquely identifies a Java
// object within a backend session.  It is valid for the lifetime of the
// Network that produced it.
using ObjectHandle = std::int64_t;
constexpr ObjectHandle INVALID_HANDLE = 0;

} // namespace iidm
