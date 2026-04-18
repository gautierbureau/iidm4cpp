#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

class RatioTapChangerStep {
public:
    RatioTapChangerStep() = default;
    explicit RatioTapChangerStep(ObjectHandle handle, BackendProvider* backend);

    double getRho() const; // transformation ratio
    double getR()   const; // resistance deviation in %
    double getX()   const; // reactance deviation in %
    double getG()   const; // conductance deviation in %
    double getB()   const; // susceptance deviation in %

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const RatioTapChangerStep& other) const;
    bool operator!=(const RatioTapChangerStep& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
