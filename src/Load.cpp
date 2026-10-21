#include <iidm/Load.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

iidm::optional<LoadModel> Load::getModel() const {
    return detail::wrapOptional<LoadModel>(backend_->getRelated(handle_, LoadModel::TYPE), backend_);
}

void Load::applySolvedValues() {
    backend_->invoke(handle_, prop::ACT_APPLY_SOLVED_VALUES);
}

void Load::setP0ToP() {
    backend_->invoke(handle_, prop::ACT_SET_TARGET_P_TO_P);
}

void Load::setQ0ToQ() {
    backend_->invoke(handle_, prop::ACT_SET_TARGET_Q_TO_Q);
}

} // namespace iidm
