#pragma once

#include <iidm/Adder.h>
#include <iidm/Ground.h>

namespace iidm {

// New ground of a voltage level (VoltageLevel::newGround())
class GroundAdder : public detail::InjectionAdder<GroundAdder> {
public:
    GroundAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    Ground add() { return Ground(create(), backend_); }
};

} // namespace iidm
