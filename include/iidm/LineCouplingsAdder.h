#pragma once

#include <iidm/Adder.h>
#include <iidm/LineCouplings.h>

namespace iidm {

// New LineCouplings extension of a network (newLineCouplings()): empty; its mutual couplings are
// added to it afterwards (LineCouplings::newMutualCoupling())
class LineCouplingsAdder : public detail::AdderBase<LineCouplingsAdder> {
public:
    LineCouplingsAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    LineCouplings add() { return LineCouplings(create(), backend_); }
};

} // namespace iidm
