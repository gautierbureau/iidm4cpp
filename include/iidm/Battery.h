#pragma once

#include <iidm/Injection.h>
#include <iidm/ReactiveLimitsHolder.h>
#include <iidm/VoltageRegulationHolder.h>

namespace iidm {

class Battery : public Injection,
                public detail::VoltageRegulationHolderMethods<Battery>,
                public detail::ReactiveLimitsHolderMethods<Battery> {
public:
    static constexpr int TYPE = prop::BATTERY;

    Battery() = default;
    explicit Battery(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getTargetP() const;
    Battery& setTargetP(double targetP);

    double getMinP() const;
    double getMaxP() const;
};

} // namespace iidm
