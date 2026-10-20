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
    double getX() const;
    double getG1() const;
    double getB1() const;
    double getG2() const;
    double getB2() const;
};

} // namespace iidm
