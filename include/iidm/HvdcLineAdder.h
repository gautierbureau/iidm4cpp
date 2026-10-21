#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/HvdcLine.h>
#include <string>

namespace iidm {

// New HVDC line of a network (Network::newHvdcLine()), between two converter stations
class HvdcLineAdder : public detail::IdentifiableAdder<HvdcLineAdder> {
public:
    HvdcLineAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder(adder, backend) {}

    HvdcLineAdder& setR(double r)               { return doubleField(prop::HVDC_R, r); }
    HvdcLineAdder& setNominalV(double nominalV) { return doubleField(prop::HVDC_NOMINAL_V, nominalV); }
    HvdcLineAdder& setActivePowerSetpoint(double activePowerSetpoint) {
        return doubleField(prop::HVDC_ACTIVE_POWER_SETPOINT, activePowerSetpoint);
    }
    HvdcLineAdder& setMaxP(double maxP)         { return doubleField(prop::HVDC_MAX_P, maxP); }
    HvdcLineAdder& setConvertersMode(HvdcConverterStationMode mode) {
        return intField(prop::HVDC_CONVERTERS_MODE, static_cast<int>(mode));
    }
    HvdcLineAdder& setConverterStationId1(const std::string& id) { return stringField(prop::HVDC_CONVERTER_STATION_ID1, id); }
    HvdcLineAdder& setConverterStationId2(const std::string& id) { return stringField(prop::HVDC_CONVERTER_STATION_ID2, id); }

    HvdcLine add() { return HvdcLine(create(), backend_); }
};

} // namespace iidm
