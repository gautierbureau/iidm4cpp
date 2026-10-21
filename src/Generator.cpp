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

bool Generator::hasGeneratorShortCircuit() const {
    return detail::hasExtension<GeneratorShortCircuit>(backend_, handle_);
}

GeneratorShortCircuit Generator::getGeneratorShortCircuit() const {
    return detail::getExtension<GeneratorShortCircuit>(backend_, handle_, "GeneratorShortCircuit");
}

GeneratorShortCircuitAdder Generator::newGeneratorShortCircuit() {
    return detail::newExtension<GeneratorShortCircuit, GeneratorShortCircuitAdder>(backend_, handle_);
}

void Generator::removeGeneratorShortCircuit() {
    detail::removeExtension<GeneratorShortCircuit>(backend_, handle_);
}

bool Generator::hasGeneratorEntsoeCategory() const {
    return detail::hasExtension<GeneratorEntsoeCategory>(backend_, handle_);
}

GeneratorEntsoeCategory Generator::getGeneratorEntsoeCategory() const {
    return detail::getExtension<GeneratorEntsoeCategory>(backend_, handle_, "GeneratorEntsoeCategory");
}

GeneratorEntsoeCategoryAdder Generator::newGeneratorEntsoeCategory() {
    return detail::newExtension<GeneratorEntsoeCategory, GeneratorEntsoeCategoryAdder>(backend_, handle_);
}

void Generator::removeGeneratorEntsoeCategory() {
    detail::removeExtension<GeneratorEntsoeCategory>(backend_, handle_);
}

bool Generator::hasGeneratorStartup() const {
    return detail::hasExtension<GeneratorStartup>(backend_, handle_);
}

GeneratorStartup Generator::getGeneratorStartup() const {
    return detail::getExtension<GeneratorStartup>(backend_, handle_, "GeneratorStartup");
}

GeneratorStartupAdder Generator::newGeneratorStartup() {
    return detail::newExtension<GeneratorStartup, GeneratorStartupAdder>(backend_, handle_);
}

void Generator::removeGeneratorStartup() {
    detail::removeExtension<GeneratorStartup>(backend_, handle_);
}

bool Generator::hasGeneratorFortescue() const {
    return detail::hasExtension<GeneratorFortescue>(backend_, handle_);
}

GeneratorFortescue Generator::getGeneratorFortescue() const {
    return detail::getExtension<GeneratorFortescue>(backend_, handle_, "GeneratorFortescue");
}

GeneratorFortescueAdder Generator::newGeneratorFortescue() {
    return detail::newExtension<GeneratorFortescue, GeneratorFortescueAdder>(backend_, handle_);
}

void Generator::removeGeneratorFortescue() {
    detail::removeExtension<GeneratorFortescue>(backend_, handle_);
}

} // namespace iidm
