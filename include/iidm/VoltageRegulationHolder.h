#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageRegulation.h>
#include <iidm/VoltageRegulationAdder.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {
namespace detail {

// What every equipment that can regulate voltage or reactive power has (powsybl 7.4
// regulation.VoltageRegulationHolder): generators, batteries, static var compensators, shunt
// compensators, VSC converter stations, ratio tap changers.
//
// Local targets: the target V of a local voltage regulation, the target Q when not regulating or
// regulating Q locally. The regulation itself (mode, remote terminal and its target value...) is a
// VoltageRegulation object.
template <class Self>
class VoltageRegulationHolderMethods {
public:
    double getLocalTargetV() const;
    Self& setLocalTargetV(double localTargetV);
    double getLocalTargetQ() const;
    Self& setLocalTargetQ(double localTargetQ);

    // Empty when it has no voltage regulation
    iidm::optional<VoltageRegulation> getVoltageRegulation() const;
    // A new voltage regulation, which replaces the current one at build()
    VoltageRegulationBuilder newVoltageRegulation();
    void removeVoltageRegulation();

    // Has a voltage regulation, and it is regulating
    bool isRegulating() const;
    // Regulating in this mode (no regulation counts as REACTIVE_POWER)
    bool isRegulatingWithMode(RegulationMode mode) const;
    // Its regulation has this mode, regulating or not (no regulation counts as REACTIVE_POWER)
    bool isWithMode(RegulationMode mode) const;

    // Target of the regulation if it regulates V (or Q) remotely, else the local target
    double getRegulatingTargetV() const;
    double getRegulatingTargetQ() const;
    // Terminal of a remote regulation, else its own terminal
    Terminal getRegulatingTerminal() const;
    // Whether the regulation has a terminal (possibly its own)
    bool hasRegulatingTerminal() const;

    // The targets from the solved values: local target Q (V) from the Q (V) of its terminal, and the
    // target value of a remote reactive power (voltage) regulation from the remote terminal
    void setTargetQToQ();
    void setTargetVToV();

protected:
    VoltageRegulationHolderMethods() = default;
    ~VoltageRegulationHolderMethods() = default;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
};

} // namespace detail
} // namespace iidm
