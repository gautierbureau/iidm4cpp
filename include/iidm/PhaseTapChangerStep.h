#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

class PhaseTapChangerStep {
public:
    PhaseTapChangerStep() = default;
    explicit PhaseTapChangerStep(ObjectHandle handle, BackendProvider* backend);
    PhaseTapChangerStep(double alpha, double rho, double r, double x, double g, double b);

    double getAlpha() const;
    double getRho()   const;
    double getR()     const;
    double getX()     const;
    double getG()     const;
    double getB()     const;

    bool isValid() const { return standalone_ || handle_ != INVALID_HANDLE; }

    bool operator==(const PhaseTapChangerStep& other) const;
    bool operator!=(const PhaseTapChangerStep& other) const;

private:
    bool             standalone_ = false;
    double           alpha_ = 0.0, rho_ = 0.0, r_ = 0.0, x_ = 0.0, g_ = 0.0, b_ = 0.0;
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
