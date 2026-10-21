#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The LineFortescue extension of a line: its zero-sequence impedance and admittances, its
// open phases, for asymmetrical studies. An object of its own, from Line::getLineFortescue().
class LineFortescue : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::LINE_FORTESCUE;

    LineFortescue() = default;
    explicit LineFortescue(ObjectHandle handle, BackendProvider* backend);

    // Zero-sequence resistance (Ω)
    double getRz() const { return getDouble(prop::EXT_LINE_F_RZ); }
    LineFortescue& setRz(double rz) { return setDouble(*this, prop::EXT_LINE_F_RZ, rz); }

    // Zero-sequence reactance (Ω)
    double getXz() const { return getDouble(prop::EXT_LINE_F_XZ); }
    LineFortescue& setXz(double xz) { return setDouble(*this, prop::EXT_LINE_F_XZ, xz); }

    // Zero-sequence shunt conductance of side 1 (S)
    double getG1z() const { return getDouble(prop::EXT_LINE_F_G1Z); }
    LineFortescue& setG1z(double g1z) { return setDouble(*this, prop::EXT_LINE_F_G1Z, g1z); }

    // Zero-sequence shunt susceptance of side 1 (S)
    double getB1z() const { return getDouble(prop::EXT_LINE_F_B1Z); }
    LineFortescue& setB1z(double b1z) { return setDouble(*this, prop::EXT_LINE_F_B1Z, b1z); }

    // Zero-sequence shunt conductance of side 2 (S)
    double getG2z() const { return getDouble(prop::EXT_LINE_F_G2Z); }
    LineFortescue& setG2z(double g2z) { return setDouble(*this, prop::EXT_LINE_F_G2Z, g2z); }

    // Zero-sequence shunt susceptance of side 2 (S)
    double getB2z() const { return getDouble(prop::EXT_LINE_F_B2Z); }
    LineFortescue& setB2z(double b2z) { return setDouble(*this, prop::EXT_LINE_F_B2Z, b2z); }

    // Whether phase A is open
    bool isOpenPhaseA() const { return getBool(prop::EXT_LINE_F_OPEN_PHASE_A); }
    LineFortescue& setOpenPhaseA(bool openPhaseA) { return setBool(*this, prop::EXT_LINE_F_OPEN_PHASE_A, openPhaseA); }

    bool isOpenPhaseB() const { return getBool(prop::EXT_LINE_F_OPEN_PHASE_B); }
    LineFortescue& setOpenPhaseB(bool openPhaseB) { return setBool(*this, prop::EXT_LINE_F_OPEN_PHASE_B, openPhaseB); }

    bool isOpenPhaseC() const { return getBool(prop::EXT_LINE_F_OPEN_PHASE_C); }
    LineFortescue& setOpenPhaseC(bool openPhaseC) { return setBool(*this, prop::EXT_LINE_F_OPEN_PHASE_C, openPhaseC); }
};

} // namespace iidm
