#pragma once

#include <iidm/Enums.h>
#include <iidm/HvdcAngleDroopActivePowerControl.h>
#include <iidm/HvdcAngleDroopActivePowerControlAdder.h>
#include <iidm/HvdcConverterStation.h>
#include <iidm/HvdcOperatorActivePowerRange.h>
#include <iidm/HvdcOperatorActivePowerRangeAdder.h>
#include <iidm/Identifiable.h>

namespace iidm {

class HvdcLine : public Identifiable {
public:
    static constexpr int TYPE = prop::HVDC_LINE;

    HvdcLine() = default;
    explicit HvdcLine(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    double getR() const;
    double getNominalV() const;
    double getActivePowerSetpoint() const;
    HvdcLine& setActivePowerSetpoint(double value);
    double getMaxP() const;

    HvdcConverterStationMode getConvertersMode() const;
    HvdcLine& setConvertersMode(HvdcConverterStationMode mode);

    HvdcConverterStation getConverterStation1() const;
    HvdcConverterStation getConverterStation2() const;

    // Removes the HVDC line (its converter stations stay)
    void remove();

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasHvdcAngleDroopActivePowerControl() const;
    HvdcAngleDroopActivePowerControl getHvdcAngleDroopActivePowerControl() const;
    HvdcAngleDroopActivePowerControlAdder newHvdcAngleDroopActivePowerControl();
    void removeHvdcAngleDroopActivePowerControl();

    bool hasHvdcOperatorActivePowerRange() const;
    HvdcOperatorActivePowerRange getHvdcOperatorActivePowerRange() const;
    HvdcOperatorActivePowerRangeAdder newHvdcOperatorActivePowerRange();
    void removeHvdcOperatorActivePowerRange();
};

} // namespace iidm
