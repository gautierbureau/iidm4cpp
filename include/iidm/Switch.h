#pragma once

#include <iidm/Enums.h>
#include <iidm/Identifiable.h>

namespace iidm {

class VoltageLevel;

class Switch : public Identifiable {
public:
    static constexpr int TYPE = prop::SWITCH;

    Switch() = default;
    explicit Switch(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    bool isOpen() const { return getBool(prop::SW_OPEN); }
    Switch& setOpen(bool open) { return setBool(*this, prop::SW_OPEN, open); }

    bool isRetained() const { return getBool(prop::SW_RETAINED); }
    Switch& setRetained(bool retained) { return setBool(*this, prop::SW_RETAINED, retained); }

    SwitchKind getKind() const { return static_cast<SwitchKind>(getInt(prop::SW_KIND)); }

    int getNode1() const { return getInt(prop::SW_NODE1); }
    int getNode2() const { return getInt(prop::SW_NODE2); }

    VoltageLevel getVoltageLevel() const;
};

} // namespace iidm
