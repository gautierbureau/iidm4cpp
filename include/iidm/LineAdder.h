#pragma once

#include <iidm/Adder.h>
#include <iidm/Line.h>

namespace iidm {

// New line of a network (Network::newLine()), between two voltage levels
class LineAdder : public detail::BranchAdder<LineAdder> {
public:
    LineAdder(ObjectHandle adder, BackendProvider* backend) : BranchAdder(adder, backend) {}

    LineAdder& setR(double r)   { return doubleField(prop::LINE_R, r); }
    LineAdder& setX(double x)   { return doubleField(prop::LINE_X, x); }
    LineAdder& setG1(double g1) { return doubleField(prop::LINE_G1, g1); }
    LineAdder& setB1(double b1) { return doubleField(prop::LINE_B1, b1); }
    LineAdder& setG2(double g2) { return doubleField(prop::LINE_G2, g2); }
    LineAdder& setB2(double b2) { return doubleField(prop::LINE_B2, b2); }

    // Sets the values and voltage levels of a line on an adder (powsybl LineAdder.fillLineAdder)
    static LineAdder& fillLineAdder(LineAdder& adder, const Line& line) {
        return adder.relatedField(prop::FILL_FROM, detail::Access::handle(line));
    }

    Line add() { return Line(create(), backend_); }
};

} // namespace iidm
