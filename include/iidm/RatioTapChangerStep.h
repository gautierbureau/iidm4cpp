#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

class RatioTapChangerStep : public detail::ObjectRef {
public:
    RatioTapChangerStep() = default;
    explicit RatioTapChangerStep(ObjectHandle handle, BackendProvider* backend);
    RatioTapChangerStep(double rho, double r, double x, double g, double b);

    double getRho() const;
    double getR()   const;
    double getX()   const;
    double getG()   const;
    double getB()   const;

    bool isValid() const { return standalone_ || handle_ != INVALID_HANDLE; }

private:
    bool             standalone_ = false;
    double           rho_ = 0.0, r_ = 0.0, x_ = 0.0, g_ = 0.0, b_ = 0.0;
};

} // namespace iidm
