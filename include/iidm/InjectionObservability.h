#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/ObservabilityQuality.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl InjectionObservability extension attached to an injection
// (Injection::getInjectionObservability()).
class InjectionObservability : public detail::ObjectRef {
public:
    InjectionObservability() = default;
    explicit InjectionObservability(ObjectHandle handle, BackendProvider* backend);

    bool isObservable() const;
    InjectionObservability& setObservable(bool observable);

    // Empty when the quality is not defined.
    iidm::optional<ObservabilityQuality> getQualityP() const;
    iidm::optional<ObservabilityQuality> getQualityQ() const;
    iidm::optional<ObservabilityQuality> getQualityV() const;

    // Sets the standard deviation, creating the quality if needed (an existing redundancy is kept).
    InjectionObservability& setQualityP(double standardDeviation);
    InjectionObservability& setQualityQ(double standardDeviation);
    InjectionObservability& setQualityV(double standardDeviation);

    // Sets the standard deviation and the redundancy (empty = undefined).
    InjectionObservability& setQualityP(double standardDeviation, iidm::optional<bool> redundant);
    InjectionObservability& setQualityQ(double standardDeviation, iidm::optional<bool> redundant);
    InjectionObservability& setQualityV(double standardDeviation, iidm::optional<bool> redundant);
};

} // namespace iidm
