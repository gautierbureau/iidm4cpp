#pragma once

#include <iidm/BusbarSectionPosition.h>
#include <iidm/BusbarSectionPositionAdder.h>
#include <iidm/Injection.h>

namespace iidm {

class BusbarSection : public Injection {
public:
    static constexpr int TYPE = prop::BUSBAR_SECTION;

    BusbarSection() = default;
    explicit BusbarSection(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getV() const;
    double getAngle() const;

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasBusbarSectionPosition() const;
    BusbarSectionPosition getBusbarSectionPosition() const;
    BusbarSectionPositionAdder newBusbarSectionPosition();
    void removeBusbarSectionPosition();
};

} // namespace iidm
