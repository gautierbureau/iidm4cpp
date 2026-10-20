#pragma once

#include <iidm/Adder.h>
#include <iidm/SlackTerminal.h>

namespace iidm {

// New SlackTerminal extension of a voltage level (newSlackTerminal()), with the slack terminal
// of the current variant
class SlackTerminalAdder : public detail::AdderBase<SlackTerminalAdder> {
public:
    SlackTerminalAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    SlackTerminalAdder& withTerminal(const Terminal& terminal) {
        return relatedField(prop::REL_TERMINAL, detail::Access::handle(terminal));
    }

    SlackTerminal add() { return SlackTerminal(create(), backend_); }
};

} // namespace iidm
