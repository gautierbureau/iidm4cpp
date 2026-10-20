#pragma once

#include <iidm/stdcxx/optional.hpp>

namespace iidm {

// Quality of an observable quantity (P, Q, V...) in the PowSyBl InjectionObservability and
// BranchObservability extensions. Plain value: reading it does not keep a link to the network.
struct ObservabilityQuality {
    double                 standardDeviation = 0.0;
    iidm::optional<bool>   redundant;   // empty when the redundancy is undefined
};

} // namespace iidm
