#include <iidm/Load.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include "Extensions.h"
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

bool Load::hasLoadDetail() const {
    return detail::hasExtension<LoadDetail>(backend_, handle_);
}

LoadDetail Load::getLoadDetail() const {
    return detail::getExtension<LoadDetail>(backend_, handle_, "LoadDetail");
}

LoadDetailAdder Load::newLoadDetail() {
    return detail::newExtension<LoadDetail, LoadDetailAdder>(backend_, handle_);
}

void Load::removeLoadDetail() {
    detail::removeExtension<LoadDetail>(backend_, handle_);
}

bool Load::hasLoadAsymmetrical() const {
    return detail::hasExtension<LoadAsymmetrical>(backend_, handle_);
}

LoadAsymmetrical Load::getLoadAsymmetrical() const {
    return detail::getExtension<LoadAsymmetrical>(backend_, handle_, "LoadAsymmetrical");
}

LoadAsymmetricalAdder Load::newLoadAsymmetrical() {
    return detail::newExtension<LoadAsymmetrical, LoadAsymmetricalAdder>(backend_, handle_);
}

void Load::removeLoadAsymmetrical() {
    detail::removeExtension<LoadAsymmetrical>(backend_, handle_);
}

} // namespace iidm
