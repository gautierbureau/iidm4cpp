#pragma once

// Property codes exchanged with the Java bridge, as iidm::prop::NAME constants.
// Defined in PropertyCodes.def — the single source of truth, also used to generate
// the Java PropertyCodes class: add or change codes there.

namespace iidm { namespace prop {

#define IIDM_CODE(name, value) constexpr int name = value;
#include <iidm/PropertyCodes.def>
#undef IIDM_CODE

} } // namespace iidm::prop
