#pragma once

#include <iidm/ReactiveLimits.h>

namespace iidm {

// Reactive limits that do not depend on the active power
class MinMaxReactiveLimits : public ReactiveLimits {
public:
    static constexpr int TYPE = prop::MIN_MAX_REACTIVE_LIMITS;

    MinMaxReactiveLimits() = default;
    explicit MinMaxReactiveLimits(ObjectHandle handle, BackendProvider* backend) : ReactiveLimits(handle, backend) {}

    double getMinQ() const { return getDouble(prop::RL_MIN_Q); }
    double getMaxQ() const { return getDouble(prop::RL_MAX_Q); }
    using ReactiveLimits::getMinQ;
    using ReactiveLimits::getMaxQ;
};

} // namespace iidm
