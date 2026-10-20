#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class BackendProvider;

// The quality of an observed quantity (P, Q, V...) of an InjectionObservability or
// BranchObservability extension: an object of its own, from getQualityP()...
class ObservabilityQuality : public detail::ObjectRef {
public:
    ObservabilityQuality() = default;
    explicit ObservabilityQuality(ObjectHandle handle, BackendProvider* backend);

    double getStandardDeviation() const;
    ObservabilityQuality& setStandardDeviation(double standardDeviation);

    // Empty when the redundancy is undefined
    iidm::optional<bool> isRedundant() const;
    // Empty makes it undefined
    ObservabilityQuality& setRedundant(iidm::optional<bool> redundant);
};

} // namespace iidm
