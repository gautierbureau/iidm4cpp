#pragma once

#include <iidm/Adder.h>
#include <iidm/VscConverterStation.h>
#include <iidm/VoltageRegulationAdder.h>

namespace iidm {

// New VSC converter station of a voltage level (VoltageLevel::newVscConverterStation()); an HVDC line
// links two converter stations
class VscConverterStationAdder : public detail::InjectionAdder<VscConverterStationAdder> {
public:
    VscConverterStationAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    // A float in powsybl
    VscConverterStationAdder& setLossFactor(double lossFactor) { return doubleField(prop::CONVERTER_STATION_LOSS_FACTOR, lossFactor); }
    VscConverterStationAdder& setLocalTargetV(double localTargetV) { return doubleField(prop::LOCAL_TARGET_V, localTargetV); }
    VscConverterStationAdder& setLocalTargetQ(double localTargetQ) { return doubleField(prop::LOCAL_TARGET_Q, localTargetQ); }

    VoltageRegulationAdder<VscConverterStationAdder> newVoltageRegulation() {
        return VoltageRegulationAdder<VscConverterStationAdder>(backend_->invoke(adder_, prop::VOLTAGE_REGULATION), backend_, *this);
    }

    VscConverterStation add() { return VscConverterStation(create(), backend_); }
};

} // namespace iidm
