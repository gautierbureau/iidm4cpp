#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/TapChangerStep.h>

namespace iidm {

class RatioTapChangerStep : public detail::ObjectRef,
                            public detail::PropertiesHolder<RatioTapChangerStep>,
                            public detail::TapChangerStepMethods<RatioTapChangerStep> {
public:
    static constexpr int TYPE = prop::RATIO_TAP_CHANGER_STEP;

    RatioTapChangerStep() = default;
    explicit RatioTapChangerStep(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}
};

} // namespace iidm
