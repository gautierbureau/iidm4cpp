#pragma once

#include <cstdint>
#include <string>

namespace iidm { namespace detail {

// The hash of the property codes: FNV-1a of each "NAME=value\n", in the order of PropertyCodes.def,
// as the Java side computes it (PropertyDispatcher.codesHash).
std::uint64_t propertyCodesHash();

// Throws BackendNotAvailableException unless `javaHash`, the hash of the codes of the Java side
// (0 for one older than the check), is the one of this library: Java code built from other codes
// would read other properties without any error.
void checkPropertyCodes(std::uint64_t javaHash, const std::string& javaSide);

} } // namespace iidm::detail
