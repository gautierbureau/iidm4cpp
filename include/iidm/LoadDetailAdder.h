#pragma once

#include <iidm/Adder.h>
#include <iidm/LoadDetail.h>

namespace iidm {

// New LoadDetail extension of a load
class LoadDetailAdder : public detail::AdderBase<LoadDetailAdder> {
public:
    LoadDetailAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    LoadDetailAdder& withFixedActivePower(double fixedActivePower) { return doubleField(prop::EXT_LD_FIXED_ACTIVE_POWER, fixedActivePower); }
    LoadDetailAdder& withFixedReactivePower(double fixedReactivePower) { return doubleField(prop::EXT_LD_FIXED_REACTIVE_POWER, fixedReactivePower); }
    LoadDetailAdder& withVariableActivePower(double variableActivePower) { return doubleField(prop::EXT_LD_VARIABLE_ACTIVE_POWER, variableActivePower); }
    LoadDetailAdder& withVariableReactivePower(double variableReactivePower) { return doubleField(prop::EXT_LD_VARIABLE_REACTIVE_POWER, variableReactivePower); }

    LoadDetail add() { return LoadDetail(create(), backend_); }
};

} // namespace iidm
