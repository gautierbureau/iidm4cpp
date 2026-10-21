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
    Battery& setMinP(double minP);
    double getMaxP() const;
    Battery& setMaxP(double maxP);

    // ── Solved values ────────────────────────────────────────────────────
    // Its targets (P, Q) from the solved values: both; the target P (setTargetQToQ() is that of
    // the voltage regulation holder)
    void applySolvedValues();
    void setTargetPToP();
};

} // namespace iidm
