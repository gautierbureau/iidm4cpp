#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class BackendProvider;

// Voltage or reactive power regulation of a generator (powsybl 7.4 regulation.VoltageRegulation).
//
// Without a terminal the regulation is local: its target value is NaN, and the target is the
// holder's local target V (VOLTAGE and VOLTAGE_PER_REACTIVE_POWER modes) or local target Q
// (REACTIVE_POWER mode). With a terminal the regulation is remote, and the target value applies
// to that terminal. powsybl validates every change: an inconsistent one throws ValidationException.
class VoltageRegulation : public detail::ObjectRef {
public:
    VoltageRegulation() = default;
    explicit VoltageRegulation(ObjectHandle handle, BackendProvider* backend);

    // Reactive power targets use the load sign convention
    double getTargetValue() const { return getDouble(prop::VR_TARGET_VALUE); }
    VoltageRegulation& setTargetValue(double targetValue) {
        return setDouble(*this, prop::VR_TARGET_VALUE, targetValue);
    }

    double getTargetDeadband() const { return getDouble(prop::VR_TARGET_DEADBAND); }
    VoltageRegulation& setTargetDeadband(double targetDeadband) {
        return setDouble(*this, prop::VR_TARGET_DEADBAND, targetDeadband);
    }

    // VOLTAGE_PER_REACTIVE_POWER mode: lambda in U0 = U + lambda * Q
    double getSlope() const { return getDouble(prop::VR_SLOPE); }
    VoltageRegulation& setSlope(double slope) { return setDouble(*this, prop::VR_SLOPE, slope); }

    iidm::optional<RegulationMode> getMode() const;
    VoltageRegulation& setMode(RegulationMode mode) { return setInt(*this, prop::VR_MODE, static_cast<int>(mode)); }

    bool isRegulating() const { return getBool(prop::VR_REGULATING); }
    VoltageRegulation& setRegulating(bool regulating) { return setBool(*this, prop::VR_REGULATING, regulating); }

    bool isWithTerminal() const { return getBool(prop::VR_WITH_TERMINAL); }
    // Empty for a local regulation
    iidm::optional<Terminal> getTerminal() const;
    // The terminal and the target value change together. A default-constructed Terminal (and a
    // NaN target value) makes the regulation local again.
    VoltageRegulation& setTerminal(const Terminal& terminal, double targetValue);
};

} // namespace iidm
