#pragma once

#include <iidm/Adder.h>
#include <iidm/MinMaxReactiveLimits.h>

namespace iidm {

// New min-max reactive limits of a generator, battery, VSC converter station or boundary line
// generation (newMinMaxReactiveLimits()): they replace its reactive limits
class MinMaxReactiveLimitsAdder : public detail::AdderBase<MinMaxReactiveLimitsAdder> {
public:
    MinMaxReactiveLimitsAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    // MVar
    MinMaxReactiveLimitsAdder& setMinQ(double minQ) { return doubleField(prop::RL_MIN_Q, minQ); }
    MinMaxReactiveLimitsAdder& setMaxQ(double maxQ) { return doubleField(prop::RL_MAX_Q, maxQ); }

    MinMaxReactiveLimits add() { return MinMaxReactiveLimits(create(), backend_); }
};

} // namespace iidm
