#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl SlackTerminal extension attached to a VoltageLevel.
class SlackTerminal : public detail::ObjectRef {
public:
    SlackTerminal() = default;
    explicit SlackTerminal(ObjectHandle handle, BackendProvider* backend);

    // Invalid Terminal when the slack terminal is unset for the current variant.
    Terminal getTerminal() const;
    // The terminal must belong to the voltage level (ValidationException otherwise);
    // an invalid Terminal unsets it.
    SlackTerminal& setTerminal(const Terminal& terminal);
    // True when no variant has a slack terminal.
    bool isEmpty() const;
};

} // namespace iidm
