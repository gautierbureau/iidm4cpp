#include <iidm/Load.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

double Load::getP0() const {
    return backend_->getDouble(handle_, prop::LOAD_P0);
}

Load& Load::setP0(double p0) {
    backend_->setDouble(handle_, prop::LOAD_P0, p0);
    return *this;
}

double Load::getQ0() const {
    return backend_->getDouble(handle_, prop::LOAD_Q0);
}

Load& Load::setQ0(double q0) {
    backend_->setDouble(handle_, prop::LOAD_Q0, q0);
    return *this;
}

LoadType Load::getLoadType() const {
    return static_cast<LoadType>(backend_->getInt(handle_, prop::LOAD_TYPE));
}

Load& Load::setLoadType(LoadType loadType) {
    backend_->setInt(handle_, prop::LOAD_TYPE, static_cast<int>(loadType));
    return *this;
}

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
