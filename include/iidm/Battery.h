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

    double getTargetP() const { return getDouble(prop::BAT_TARGET_P); }
    Battery& setTargetP(double targetP) { return setDouble(*this, prop::BAT_TARGET_P, targetP); }

    double getMinP() const { return getDouble(prop::BAT_MIN_P); }
    Battery& setMinP(double minP) { return setDouble(*this, prop::BAT_MIN_P, minP); }
    double getMaxP() const { return getDouble(prop::BAT_MAX_P); }
    Battery& setMaxP(double maxP) { return setDouble(*this, prop::BAT_MAX_P, maxP); }

    // ── Solved values ────────────────────────────────────────────────────
    // Its targets (P, Q) from the solved values: both; the target P (setTargetQToQ() is that of
    // the voltage regulation holder)
    void applySolvedValues();
    void setTargetPToP();
};

} // namespace iidm
