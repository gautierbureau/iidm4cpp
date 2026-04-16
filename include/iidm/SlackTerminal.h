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

    Terminal getTerminal() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const SlackTerminal& other) const;
    bool operator!=(const SlackTerminal& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
