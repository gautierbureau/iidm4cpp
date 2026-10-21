#pragma once

#include <iidm/Adder.h>
#include <iidm/LineFortescue.h>

namespace iidm {

// New LineFortescue extension of a line
class LineFortescueAdder : public detail::AdderBase<LineFortescueAdder> {
public:
    LineFortescueAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    LineFortescueAdder& withRz(double rz) { return doubleField(prop::EXT_LINE_F_RZ, rz); }
    LineFortescueAdder& withXz(double xz) { return doubleField(prop::EXT_LINE_F_XZ, xz); }
    LineFortescueAdder& withG1z(double g1z) { return doubleField(prop::EXT_LINE_F_G1Z, g1z); }
    LineFortescueAdder& withB1z(double b1z) { return doubleField(prop::EXT_LINE_F_B1Z, b1z); }
    LineFortescueAdder& withG2z(double g2z) { return doubleField(prop::EXT_LINE_F_G2Z, g2z); }
    LineFortescueAdder& withB2z(double b2z) { return doubleField(prop::EXT_LINE_F_B2Z, b2z); }
    LineFortescueAdder& withOpenPhaseA(bool openPhaseA) { return boolField(prop::EXT_LINE_F_OPEN_PHASE_A, openPhaseA); }
    LineFortescueAdder& withOpenPhaseB(bool openPhaseB) { return boolField(prop::EXT_LINE_F_OPEN_PHASE_B, openPhaseB); }
    LineFortescueAdder& withOpenPhaseC(bool openPhaseC) { return boolField(prop::EXT_LINE_F_OPEN_PHASE_C, openPhaseC); }

    LineFortescue add() { return LineFortescue(create(), backend_); }
};

} // namespace iidm
