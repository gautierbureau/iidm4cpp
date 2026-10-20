#pragma once

#include <iidm/Enums.h>
#include <iidm/Identifiable.h>

namespace iidm {

class Switch : public Identifiable {
public:
    static constexpr int TYPE = prop::SWITCH;

    Switch() = default;
    explicit Switch(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    bool isOpen() const;
    Switch& setOpen(bool open);

    bool isRetained() const;
    Switch& setRetained(bool retained);

    SwitchKind getKind() const;

    int getNode1() const;
    int getNode2() const;
};

} // namespace iidm
