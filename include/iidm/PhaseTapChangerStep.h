#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/TapChangerStep.h>

namespace iidm {

class PhaseTapChangerStep : public detail::ObjectRef,
                            public detail::PropertiesHolder<PhaseTapChangerStep>,
                            public detail::TapChangerStepMethods<PhaseTapChangerStep> {
public:
    static constexpr int TYPE = prop::PHASE_TAP_CHANGER_STEP;

    PhaseTapChangerStep() = default;
    explicit PhaseTapChangerStep(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    // Phase shift (degrees)
    double getAlpha() const { return getDouble(prop::PTC_STEP_ALPHA); }
    PhaseTapChangerStep& setAlpha(double alpha) { return setDouble(*this, prop::PTC_STEP_ALPHA, alpha); }
};

} // namespace iidm
