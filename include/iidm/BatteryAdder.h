#pragma once

#include <iidm/Adder.h>
#include <iidm/Battery.h>
#include <iidm/VoltageRegulationAdder.h>

namespace iidm {

// New battery of a voltage level (VoltageLevel::newBattery()); it regulates as a generator does
class BatteryAdder : public detail::InjectionAdder<BatteryAdder> {
public:
    BatteryAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    BatteryAdder& setTargetP(double targetP)             { return doubleField(prop::BAT_TARGET_P, targetP); }
    BatteryAdder& setMinP(double minP)                   { return doubleField(prop::BAT_MIN_P, minP); }
    BatteryAdder& setMaxP(double maxP)                   { return doubleField(prop::BAT_MAX_P, maxP); }
    BatteryAdder& setLocalTargetV(double localTargetV)   { return doubleField(prop::LOCAL_TARGET_V, localTargetV); }
    BatteryAdder& setLocalTargetQ(double localTargetQ)   { return doubleField(prop::LOCAL_TARGET_Q, localTargetQ); }

    VoltageRegulationAdder<BatteryAdder> newVoltageRegulation() {
        return VoltageRegulationAdder<BatteryAdder>(backend_->invoke(adder_, prop::VOLTAGE_REGULATION), backend_, *this);
    }

    Battery add() { return Battery(create(), backend_); }
};

} // namespace iidm
