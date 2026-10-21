#pragma once

#include <iidm/Injection.h>

namespace iidm {

// A ground: an injection that connects its node or bus to the ground (powsybl Ground)
class Ground : public Injection {
public:
    static constexpr int TYPE = prop::GROUND;

    Ground() = default;
    explicit Ground(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}
};

} // namespace iidm
