#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/RatioTapChangerStep.h>
#include <iidm/TapChanger.h>
#include <iidm/VoltageRegulationHolder.h>

namespace iidm {

// The ratio tap changer of a two windings transformer or of a three windings transformer leg. It
// regulates the voltage (or the reactive power) as every regulation holder, through its
// VoltageRegulation (powsybl 7.4).
class RatioTapChanger : public detail::ObjectRef,
                        public detail::PropertiesHolder<RatioTapChanger>,
                        public detail::TapChangerMethods<RatioTapChanger, RatioTapChangerStep>,
                        public detail::VoltageRegulationHolderMethods<RatioTapChanger> {
public:
    static constexpr int TYPE = prop::RATIO_TAP_CHANGER;

    RatioTapChanger() = default;
    explicit RatioTapChanger(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    // The same method in both powsybl interfaces
    using detail::TapChangerMethods<RatioTapChanger, RatioTapChangerStep>::isRegulating;
};

} // namespace iidm
