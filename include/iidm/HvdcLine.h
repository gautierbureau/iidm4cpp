#pragma once

#include <iidm/Enums.h>
#include <iidm/HvdcConverterStation.h>
#include <iidm/Identifiable.h>

namespace iidm {

class HvdcLine : public Identifiable {
public:
    static constexpr int TYPE = prop::HVDC_LINE;

    HvdcLine() = default;
    explicit HvdcLine(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    double getR() const { return getDouble(prop::HVDC_R); }
    HvdcLine& setR(double r) { return setDouble(*this, prop::HVDC_R, r); }
    double getNominalV() const { return getDouble(prop::HVDC_NOMINAL_V); }
    HvdcLine& setNominalV(double nominalV) { return setDouble(*this, prop::HVDC_NOMINAL_V, nominalV); }
    double getActivePowerSetpoint() const { return getDouble(prop::HVDC_ACTIVE_POWER_SETPOINT); }
    HvdcLine& setActivePowerSetpoint(double value) { return setDouble(*this, prop::HVDC_ACTIVE_POWER_SETPOINT, value); }
    double getMaxP() const { return getDouble(prop::HVDC_MAX_P); }
    HvdcLine& setMaxP(double maxP) { return setDouble(*this, prop::HVDC_MAX_P, maxP); }

    HvdcConverterStationMode getConvertersMode() const {
        return static_cast<HvdcConverterStationMode>(getInt(prop::HVDC_CONVERTERS_MODE));
    }
    HvdcLine& setConvertersMode(HvdcConverterStationMode mode) {
        return setInt(*this, prop::HVDC_CONVERTERS_MODE, static_cast<int>(mode));
    }

    HvdcConverterStation getConverterStation1() const;
    HvdcConverterStation getConverterStation2() const;
    HvdcConverterStation getConverterStation(TwoSides side) const;

    // Connects (disconnects) both converter stations, operating breakers if needed: true when
    // this changed them, false when it could not, or already was
    bool connectConverterStations();
    bool disconnectConverterStations();
    // The same, operating only the switches the predicate accepts (see Terminal), on both sides or
    // on one
    bool connectConverterStations(SwitchPredicate switches);
    bool connectConverterStations(SwitchPredicate switches, TwoSides side);
    bool disconnectConverterStations(SwitchPredicate switches);
    bool disconnectConverterStations(SwitchPredicate switches, TwoSides side);

    // Removes the HVDC line (its converter stations stay)
    void remove();
};

} // namespace iidm
