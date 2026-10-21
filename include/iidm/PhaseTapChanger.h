#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/PhaseTapChangerStep.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/TapChanger.h>
#include <iidm/TapChangerStepsReplacer.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

// The phase tap changer of a two windings transformer or of a three windings transformer leg: it
// limits the current, or controls the active power, at its regulation terminal.
class PhaseTapChanger : public detail::ObjectRef,
                        public detail::PropertiesHolder<PhaseTapChanger>,
                        public detail::TapChangerMethods<PhaseTapChanger, PhaseTapChangerStep, PhaseTapChangerStepsReplacer> {
public:
    static constexpr int TYPE = prop::PHASE_TAP_CHANGER;

    PhaseTapChanger() = default;
    explicit PhaseTapChanger(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    PhaseTapChanger& setRegulating(bool regulating) { return setBool(*this, prop::REGULATING, regulating); }

    PhaseTapChangerRegulationMode getRegulationMode() const {
        return static_cast<PhaseTapChangerRegulationMode>(getInt(prop::PTC_REGULATION_MODE));
    }
    PhaseTapChanger& setRegulationMode(PhaseTapChangerRegulationMode regulationMode) {
        return setInt(*this, prop::PTC_REGULATION_MODE, static_cast<int>(regulationMode));
    }

    // Current (A) or active power (MW) target, after the regulation mode
    double getRegulationValue() const { return getDouble(prop::PTC_REGULATION_VALUE); }
    PhaseTapChanger& setRegulationValue(double regulationValue) {
        return setDouble(*this, prop::PTC_REGULATION_VALUE, regulationValue);
    }

    // NaN when undefined
    double getTargetDeadband() const { return getDouble(prop::PTC_TARGET_DEADBAND); }
    PhaseTapChanger& setTargetDeadband(double targetDeadband) {
        return setDouble(*this, prop::PTC_TARGET_DEADBAND, targetDeadband);
    }

    // Empty when it has none
    iidm::optional<Terminal> getRegulationTerminal() const;
    PhaseTapChanger& setRegulationTerminal(const Terminal& regulationTerminal);
};

} // namespace iidm
