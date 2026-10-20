#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl SlackTerminal extension attached to a VoltageLevel.
class SlackTerminal {
public:
    SlackTerminal() = default;
    explicit SlackTerminal(ObjectHandle handle, BackendProvider* backend);

    // Invalid Terminal when the slack terminal is unset for the current variant.
    Terminal getTerminal() const;
    // The terminal must belong to the voltage level (powsybl-core rejects any other);
    // an invalid Terminal unsets it.
    SlackTerminal& setTerminal(const Terminal& terminal);
    // True when no variant has a slack terminal.
    bool isEmpty() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const SlackTerminal& other) const;
    bool operator!=(const SlackTerminal& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
