#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <string>

namespace iidm {

class BackendProvider;

class Switch {
public:
    Switch() = default;
    explicit Switch(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    bool isOpen() const;
    Switch& setOpen(bool open);

    bool isRetained() const;
    Switch& setRetained(bool retained);

    SwitchKind getKind() const;

    int getNode1() const;
    int getNode2() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Switch& other) const;
    bool operator!=(const Switch& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
