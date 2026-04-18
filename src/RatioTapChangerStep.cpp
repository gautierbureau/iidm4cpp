#include <iidm/RatioTapChangerStep.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

RatioTapChangerStep::RatioTapChangerStep(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

double RatioTapChangerStep::getRho() const { return backend_->getDouble(handle_, prop::RTC_STEP_RHO); }
double RatioTapChangerStep::getR()   const { return backend_->getDouble(handle_, prop::RTC_STEP_R);   }
double RatioTapChangerStep::getX()   const { return backend_->getDouble(handle_, prop::RTC_STEP_X);   }
double RatioTapChangerStep::getG()   const { return backend_->getDouble(handle_, prop::RTC_STEP_G);   }
double RatioTapChangerStep::getB()   const { return backend_->getDouble(handle_, prop::RTC_STEP_B);   }

bool RatioTapChangerStep::operator==(const RatioTapChangerStep& other) const {
    return handle_ == other.handle_;
}
bool RatioTapChangerStep::operator!=(const RatioTapChangerStep& other) const {
    return !(*this == other);
}

} // namespace iidm
