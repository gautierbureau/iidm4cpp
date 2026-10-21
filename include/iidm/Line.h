#pragma once

#include <iidm/Branch.h>
#include <iidm/Connectable.h>
#include <iidm/LineFortescue.h>
#include <iidm/LineFortescueAdder.h>
#include <iidm/LinePosition.h>
#include <iidm/LinePositionAdder.h>

namespace iidm {

class Line : public Connectable, public detail::BranchMethods<Line> {
public:
    static constexpr int TYPE = prop::LINE;

    Line() = default;
    explicit Line(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    double getR() const { return getDouble(prop::LINE_R); }
    Line& setR(double r) { return setDouble(*this, prop::LINE_R, r); }
    double getX() const { return getDouble(prop::LINE_X); }
    Line& setX(double x) { return setDouble(*this, prop::LINE_X, x); }
    double getG1() const { return getDouble(prop::LINE_G1); }
    Line& setG1(double g1) { return setDouble(*this, prop::LINE_G1, g1); }
    double getB1() const { return getDouble(prop::LINE_B1); }
    Line& setB1(double b1) { return setDouble(*this, prop::LINE_B1, b1); }
    double getG2() const { return getDouble(prop::LINE_G2); }
    Line& setG2(double g2) { return setDouble(*this, prop::LINE_G2, g2); }
    double getB2() const { return getDouble(prop::LINE_B2); }
    Line& setB2(double b2) { return setDouble(*this, prop::LINE_B2, b2); }

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasLineFortescue() const;
    LineFortescue getLineFortescue() const;
    LineFortescueAdder newLineFortescue();
    void removeLineFortescue();

    bool hasLinePosition() const;
    LinePosition getLinePosition() const;
    LinePositionAdder newLinePosition();
    void removeLinePosition();
};

} // namespace iidm
