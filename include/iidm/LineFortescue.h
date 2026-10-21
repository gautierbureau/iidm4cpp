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
    double getRz() const;
    LineFortescue& setRz(double rz);

    // Zero-sequence reactance (Ω)
    double getXz() const;
    LineFortescue& setXz(double xz);

    // Zero-sequence shunt conductance of side 1 (S)
    double getG1z() const;
    LineFortescue& setG1z(double g1z);

    // Zero-sequence shunt susceptance of side 1 (S)
    double getB1z() const;
    LineFortescue& setB1z(double b1z);

    // Zero-sequence shunt conductance of side 2 (S)
    double getG2z() const;
    LineFortescue& setG2z(double g2z);

    // Zero-sequence shunt susceptance of side 2 (S)
    double getB2z() const;
    LineFortescue& setB2z(double b2z);

    // Whether phase A is open
    bool isOpenPhaseA() const;
    LineFortescue& setOpenPhaseA(bool openPhaseA);

    bool isOpenPhaseB() const;
    LineFortescue& setOpenPhaseB(bool openPhaseB);

    bool isOpenPhaseC() const;
    LineFortescue& setOpenPhaseC(bool openPhaseC);
};

} // namespace iidm
