#include "PropertyCodesCheck.h"
#include <iidm/IidmException.h>
#include <cstdio>

namespace iidm { namespace detail {

std::uint64_t propertyCodesHash() {
    static const std::uint64_t hash = [] {
        std::uint64_t h = 0xcbf29ce484222325ULL;
        auto add = [&h](const std::string& code) {
            for (unsigned char c : code) {
                h ^= c;
                h *= 0x100000001b3ULL;
            }
        };
#define IIDM_CODE(NAME, VALUE) add(#NAME "=" + std::to_string(VALUE) + "\n");
#include <iidm/PropertyCodes.def>
#undef IIDM_CODE
        return h;
    }();
    return hash;
}

namespace {

std::string hex(std::uint64_t value) {
    char text[17];
    std::snprintf(text, sizeof(text), "%016llx", static_cast<unsigned long long>(value));
    return text;
}

} // namespace

void checkPropertyCodes(std::uint64_t javaHash, const std::string& javaSide) {
    const std::uint64_t hash = propertyCodesHash();
    if (javaHash != hash) {
        throw BackendNotAvailableException(javaSide + " was built from other property codes than this "
                                           "library (codes " + hex(javaHash) + ", expected " + hex(hash)
                                           + "): build both from the same sources");
    }
}

} } // namespace iidm::detail
