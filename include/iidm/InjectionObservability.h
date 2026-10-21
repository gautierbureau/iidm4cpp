#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/ObservabilityQuality.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class BackendProvider;

// The InjectionObservability extension of an injection (generator, load, busbar section...): an
// object of its own, from Injection::getInjectionObservability().
class InjectionObservability : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::INJECTION_OBSERVABILITY;

    InjectionObservability() = default;
    explicit InjectionObservability(ObjectHandle handle, BackendProvider* backend);

    bool isObservable() const { return getBool(prop::EXT_OBS_OBSERVABLE); }
    InjectionObservability& setObservable(bool observable) {
        return setBool(*this, prop::EXT_OBS_OBSERVABLE, observable);
    }

    // Empty when the quality is not defined
    iidm::optional<ObservabilityQuality> getQualityP() const;
    iidm::optional<ObservabilityQuality> getQualityQ() const;
    iidm::optional<ObservabilityQuality> getQualityV() const;

    // Sets the standard deviation, creating the quality if needed (an existing redundancy is kept)
    InjectionObservability& setQualityP(double standardDeviation);
    InjectionObservability& setQualityQ(double standardDeviation);
    InjectionObservability& setQualityV(double standardDeviation);

    // Sets the standard deviation and the redundancy (empty = undefined)
    InjectionObservability& setQualityP(double standardDeviation, iidm::optional<bool> redundant);
    InjectionObservability& setQualityQ(double standardDeviation, iidm::optional<bool> redundant);
    InjectionObservability& setQualityV(double standardDeviation, iidm::optional<bool> redundant);
};

} // namespace iidm
