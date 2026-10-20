#include <iidm/Generator.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

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

// ── Voltage regulation ───────────────────────────────────────────────────────

double Generator::getLocalTargetV() const {
    return backend_->getDouble(handle_, prop::LOCAL_TARGET_V);
}

Generator& Generator::setLocalTargetV(double localTargetV) {
    backend_->setDouble(handle_, prop::LOCAL_TARGET_V, localTargetV);
    return *this;
}

double Generator::getLocalTargetQ() const {
    return backend_->getDouble(handle_, prop::LOCAL_TARGET_Q);
}

Generator& Generator::setLocalTargetQ(double localTargetQ) {
    backend_->setDouble(handle_, prop::LOCAL_TARGET_Q, localTargetQ);
    return *this;
}

iidm::optional<VoltageRegulation> Generator::getVoltageRegulation() const {
    ObjectHandle regulation = backend_->getRelated(handle_, prop::VOLTAGE_REGULATION);
    if (regulation == INVALID_HANDLE) return {};
    return VoltageRegulation(regulation, backend_);
}

VoltageRegulationBuilder Generator::newVoltageRegulation() {
    return VoltageRegulationBuilder(backend_->invoke(handle_, prop::VOLTAGE_REGULATION), backend_);
}

void Generator::removeVoltageRegulation() {
    backend_->setRelated(handle_, prop::VOLTAGE_REGULATION, INVALID_HANDLE);
}

bool Generator::isRegulating() const {
    return backend_->getBool(handle_, prop::REGULATING);
}

double Generator::getRegulatingTargetV() const {
    return backend_->getDouble(handle_, prop::REGULATING_TARGET_V);
}

double Generator::getRegulatingTargetQ() const {
    return backend_->getDouble(handle_, prop::REGULATING_TARGET_Q);
}

Terminal Generator::getRegulatingTerminal() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_REGULATING_TERMINAL), backend_);
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool Generator::hasCoordinatedReactiveControl() const {
    return backend_->getBool(handle_, prop::EXT_CRC_EXISTS);
}

CoordinatedReactiveControl Generator::getCoordinatedReactiveControl() const {
    return CoordinatedReactiveControl(handle_, backend_);
}

bool Generator::hasMinMaxReactiveLimits() const {
    return backend_->getInt(handle_, prop::GEN_REACTIVE_LIMITS_KIND) == 1;
}

MinMaxReactiveLimits Generator::getMinMaxReactiveLimits() const {
    return MinMaxReactiveLimits(handle_, backend_, prop::GEN_MIN_Q, prop::GEN_MAX_Q);
}

bool Generator::hasReactiveCapabilityCurve() const {
    return backend_->getInt(handle_, prop::GEN_REACTIVE_LIMITS_KIND) == 2;
}

ReactiveCapabilityCurve Generator::getReactiveCapabilityCurve() const {
    return ReactiveCapabilityCurve(handle_, backend_);
}

} // namespace iidm
