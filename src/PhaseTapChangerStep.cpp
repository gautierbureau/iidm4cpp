#include <iidm/PhaseTapChangerStep.h>
#include <iidm/BackendProvider.h>

namespace iidm {

double PhaseTapChangerStep::getAlpha() const {
    return backend_->getDouble(handle_, prop::PTC_STEP_ALPHA);
}

PhaseTapChangerStep& PhaseTapChangerStep::setAlpha(double alpha) {
    backend_->setDouble(handle_, prop::PTC_STEP_ALPHA, alpha);
    return *this;
}

} // namespace iidm
