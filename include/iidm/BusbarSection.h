#pragma once

#include <iidm/Injection.h>

namespace iidm {

class BusbarSection : public Injection {
public:
    static constexpr int TYPE = prop::BUSBAR_SECTION;

    BusbarSection() = default;
    explicit BusbarSection(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getV() const;
    double getAngle() const;
};

} // namespace iidm
