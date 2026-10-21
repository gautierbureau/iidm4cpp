#pragma once

#include <iidm/Adder.h>
#include <iidm/LccConverterStation.h>

namespace iidm {

// New LCC converter station of a voltage level (VoltageLevel::newLccConverterStation()); an HVDC line
// links two converter stations
class LccConverterStationAdder : public detail::InjectionAdder<LccConverterStationAdder> {
public:
    LccConverterStationAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    // Floats in powsybl
    LccConverterStationAdder& setLossFactor(double lossFactor) { return doubleField(prop::CONVERTER_STATION_LOSS_FACTOR, lossFactor); }
    LccConverterStationAdder& setPowerFactor(double powerFactor) { return doubleField(prop::LCC_POWER_FACTOR, powerFactor); }

    LccConverterStation add() { return LccConverterStation(create(), backend_); }
};

} // namespace iidm
