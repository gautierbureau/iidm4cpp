#include <iidm/PhaseTapChangerStep.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

PhaseTapChangerStep::PhaseTapChangerStep(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

PhaseTapChangerStep::PhaseTapChangerStep(double alpha, double rho, double r, double x, double g, double b)
    : standalone_(true), alpha_(alpha), rho_(rho), r_(r), x_(x), g_(g), b_(b) {}

double PhaseTapChangerStep::getAlpha() const { return standalone_ ? alpha_ : backend_->getDouble(handle_, prop::PTC_STEP_ALPHA); }
double PhaseTapChangerStep::getRho()   const { return standalone_ ? rho_   : backend_->getDouble(handle_, prop::PTC_STEP_RHO);   }
double PhaseTapChangerStep::getR()     const { return standalone_ ? r_     : backend_->getDouble(handle_, prop::PTC_STEP_R);     }
double PhaseTapChangerStep::getX()     const { return standalone_ ? x_     : backend_->getDouble(handle_, prop::PTC_STEP_X);     }
double PhaseTapChangerStep::getG()     const { return standalone_ ? g_     : backend_->getDouble(handle_, prop::PTC_STEP_G);     }
double PhaseTapChangerStep::getB()     const { return standalone_ ? b_     : backend_->getDouble(handle_, prop::PTC_STEP_B);     }

bool PhaseTapChangerStep::operator==(const PhaseTapChangerStep& other) const {
    return handle_ == other.handle_;
}
bool PhaseTapChangerStep::operator!=(const PhaseTapChangerStep& other) const {
    return !(*this == other);
}

} // namespace iidm
