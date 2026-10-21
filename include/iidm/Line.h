#pragma once

#include <iidm/Branch.h>
#include <iidm/Connectable.h>

namespace iidm {

class Line : public Connectable, public detail::BranchMethods<Line> {
public:
    static constexpr int TYPE = prop::LINE;

    Line() = default;
    explicit Line(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    double getR() const;
    Line& setR(double r);
    double getX() const;
    Line& setX(double x);
    double getG1() const;
    Line& setG1(double g1);
    double getB1() const;
    Line& setB1(double b1);
    double getG2() const;
    Line& setG2(double g2);
    double getB2() const;
    Line& setB2(double b2);
};

} // namespace iidm
