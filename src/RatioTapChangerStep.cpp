#include <iidm/RatioTapChangerStep.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

RatioTapChangerStep::RatioTapChangerStep(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

RatioTapChangerStep::RatioTapChangerStep(double rho, double r, double x, double g, double b)
    : standalone_(true), rho_(rho), r_(r), x_(x), g_(g), b_(b) {}

double RatioTapChangerStep::getRho() const { return standalone_ ? rho_ : backend_->getDouble(handle_, prop::RTC_STEP_RHO); }
double RatioTapChangerStep::getR()   const { return standalone_ ? r_   : backend_->getDouble(handle_, prop::RTC_STEP_R);   }
double RatioTapChangerStep::getX()   const { return standalone_ ? x_   : backend_->getDouble(handle_, prop::RTC_STEP_X);   }
double RatioTapChangerStep::getG()   const { return standalone_ ? g_   : backend_->getDouble(handle_, prop::RTC_STEP_G);   }
double RatioTapChangerStep::getB()   const { return standalone_ ? b_   : backend_->getDouble(handle_, prop::RTC_STEP_B);   }

bool RatioTapChangerStep::operator==(const RatioTapChangerStep& other) const {
    return handle_ == other.handle_;
}
bool RatioTapChangerStep::operator!=(const RatioTapChangerStep& other) const {
    return !(*this == other);
}

} // namespace iidm
