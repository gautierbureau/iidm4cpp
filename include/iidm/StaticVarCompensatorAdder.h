#pragma once

#include <iidm/Adder.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/VoltageRegulationAdder.h>

namespace iidm {

// New static var compensator of a voltage level (VoltageLevel::newStaticVarCompensator())
class StaticVarCompensatorAdder : public detail::InjectionAdder<StaticVarCompensatorAdder> {
public:
    StaticVarCompensatorAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    // Susceptances (S)
    StaticVarCompensatorAdder& setBmin(double bMin) { return doubleField(prop::SVC_B_MIN, bMin); }
    StaticVarCompensatorAdder& setBmax(double bMax) { return doubleField(prop::SVC_B_MAX, bMax); }
    StaticVarCompensatorAdder& setLocalTargetV(double localTargetV) { return doubleField(prop::LOCAL_TARGET_V, localTargetV); }
    StaticVarCompensatorAdder& setLocalTargetQ(double localTargetQ) { return doubleField(prop::LOCAL_TARGET_Q, localTargetQ); }

    VoltageRegulationAdder<StaticVarCompensatorAdder> newVoltageRegulation() {
        return VoltageRegulationAdder<StaticVarCompensatorAdder>(backend_->invoke(adder_, prop::VOLTAGE_REGULATION), backend_, *this);
    }

    StaticVarCompensator add() { return StaticVarCompensator(create(), backend_); }
};

} // namespace iidm
