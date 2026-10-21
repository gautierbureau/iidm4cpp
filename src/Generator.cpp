#include <iidm/Generator.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include "Extensions.h"

namespace iidm {

double Generator::getTargetP() const {
    return backend_->getDouble(handle_, prop::GEN_TARGET_P);
}

Generator& Generator::setTargetP(double targetP) {
    backend_->setDouble(handle_, prop::GEN_TARGET_P, targetP);
    return *this;
}

double Generator::getMinP() const {
    return backend_->getDouble(handle_, prop::GEN_MIN_P);
}

Generator& Generator::setMinP(double minP) {
    backend_->setDouble(handle_, prop::GEN_MIN_P, minP);
    return *this;
}

double Generator::getMaxP() const {
    return backend_->getDouble(handle_, prop::GEN_MAX_P);
}

Generator& Generator::setMaxP(double maxP) {
    backend_->setDouble(handle_, prop::GEN_MAX_P, maxP);
    return *this;
}

double Generator::getRatedS() const {
    return backend_->getDouble(handle_, prop::GEN_RATED_S);
}

Generator& Generator::setRatedS(double ratedS) {
    backend_->setDouble(handle_, prop::GEN_RATED_S, ratedS);
    return *this;
}

bool Generator::isCondenser() const {
    return backend_->getBool(handle_, prop::GEN_CONDENSER);
}

EnergySource Generator::getEnergySource() const {
    return static_cast<EnergySource>(backend_->getInt(handle_, prop::GEN_ENERGY_SOURCE));
}

Generator& Generator::setEnergySource(EnergySource energySource) {
    backend_->setInt(handle_, prop::GEN_ENERGY_SOURCE, static_cast<int>(energySource));
    return *this;
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool Generator::hasCoordinatedReactiveControl() const {
    return detail::hasExtension<CoordinatedReactiveControl>(backend_, handle_);
}

CoordinatedReactiveControl Generator::getCoordinatedReactiveControl() const {
    return detail::getExtension<CoordinatedReactiveControl>(backend_, handle_, "CoordinatedReactiveControl");
}

CoordinatedReactiveControlAdder Generator::newCoordinatedReactiveControl() {
    return detail::newExtension<CoordinatedReactiveControl, CoordinatedReactiveControlAdder>(backend_, handle_);
}

void Generator::removeCoordinatedReactiveControl() {
    detail::removeExtension<CoordinatedReactiveControl>(backend_, handle_);
}

// ── Solved values ────────────────────────────────────────────────────────────

void Generator::applySolvedValues() {
    backend_->invoke(handle_, prop::ACT_APPLY_SOLVED_VALUES);
}

void Generator::setTargetPToP() {
    backend_->invoke(handle_, prop::ACT_SET_TARGET_P_TO_P);
}

} // namespace iidm
