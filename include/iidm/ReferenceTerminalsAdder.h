#pragma once

#include <iidm/Adder.h>
#include <iidm/ReferenceTerminals.h>
#include <vector>

namespace iidm {

// New ReferenceTerminals extension of a network (newReferenceTerminals())
class ReferenceTerminalsAdder : public detail::AdderBase<ReferenceTerminalsAdder> {
public:
    ReferenceTerminalsAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    ReferenceTerminalsAdder& withTerminals(const std::vector<Terminal>& terminals) {
        for (const Terminal& terminal : terminals) relatedField(prop::ADD_REFERENCE_TERMINAL, detail::Access::handle(terminal));
        return *this;
    }

    ReferenceTerminals add() { return ReferenceTerminals(create(), backend_); }
};

} // namespace iidm
