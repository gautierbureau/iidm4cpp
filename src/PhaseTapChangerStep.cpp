#include <iidm/PhaseTapChangerStep.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

PhaseTapChangerStep::PhaseTapChangerStep(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

double PhaseTapChangerStep::getAlpha() const { return backend_->getDouble(handle_, prop::PTC_STEP_ALPHA); }
double PhaseTapChangerStep::getRho()   const { return backend_->getDouble(handle_, prop::PTC_STEP_RHO);   }
double PhaseTapChangerStep::getR()     const { return backend_->getDouble(handle_, prop::PTC_STEP_R);     }
double PhaseTapChangerStep::getX()     const { return backend_->getDouble(handle_, prop::PTC_STEP_X);     }
double PhaseTapChangerStep::getG()     const { return backend_->getDouble(handle_, prop::PTC_STEP_G);     }
double PhaseTapChangerStep::getB()     const { return backend_->getDouble(handle_, prop::PTC_STEP_B);     }

bool PhaseTapChangerStep::operator==(const PhaseTapChangerStep& other) const {
    return handle_ == other.handle_;
}
bool PhaseTapChangerStep::operator!=(const PhaseTapChangerStep& other) const {
    return !(*this == other);
}

} // namespace iidm
