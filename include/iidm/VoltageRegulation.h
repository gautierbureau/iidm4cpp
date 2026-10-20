#pragma once

#include <iidm/Enums.h>
#include <iidm/HandleRef.h>
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
class VoltageRegulation {
public:
    VoltageRegulation() = default;
    explicit VoltageRegulation(ObjectHandle handle, BackendProvider* backend);

    // Reactive power targets use the load sign convention
    double getTargetValue() const;
    VoltageRegulation& setTargetValue(double targetValue);

    double getTargetDeadband() const;
    VoltageRegulation& setTargetDeadband(double targetDeadband);

    // VOLTAGE_PER_REACTIVE_POWER mode: lambda in U0 = U + lambda * Q
    double getSlope() const;
    VoltageRegulation& setSlope(double slope);

    iidm::optional<RegulationMode> getMode() const;
    VoltageRegulation& setMode(RegulationMode mode);

    bool isRegulating() const;
    VoltageRegulation& setRegulating(bool regulating);

    bool isWithTerminal() const;
    // Empty for a local regulation
    iidm::optional<Terminal> getTerminal() const;
    // The terminal and the target value change together. A default-constructed Terminal (and a
    // NaN target value) makes the regulation local again.
    VoltageRegulation& setTerminal(const Terminal& terminal, double targetValue);

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const VoltageRegulation& other) const;
    bool operator!=(const VoltageRegulation& other) const;

private:
    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
