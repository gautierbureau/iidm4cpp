#include <iidm/BoundaryLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/VoltageLevel.h>
#include "BoundaryLineFilter.h"
#include "Wrap.h"

#include <algorithm>

namespace iidm {

double BoundaryLine::getP0() const {
    return backend_->getDouble(handle_, prop::BL_P0);
}

BoundaryLine& BoundaryLine::setP0(double p0) {
    backend_->setDouble(handle_, prop::BL_P0, p0);
    return *this;
}

double BoundaryLine::getQ0() const {
    return backend_->getDouble(handle_, prop::BL_Q0);
}

BoundaryLine& BoundaryLine::setQ0(double q0) {
    backend_->setDouble(handle_, prop::BL_Q0, q0);
    return *this;
}

double BoundaryLine::getR() const {
    return backend_->getDouble(handle_, prop::BL_R);
}

BoundaryLine& BoundaryLine::setR(double r) {
    backend_->setDouble(handle_, prop::BL_R, r);
    return *this;
}

double BoundaryLine::getX() const {
    return backend_->getDouble(handle_, prop::BL_X);
}

BoundaryLine& BoundaryLine::setX(double x) {
    backend_->setDouble(handle_, prop::BL_X, x);
    return *this;
}

double BoundaryLine::getG() const {
    return backend_->getDouble(handle_, prop::BL_G);
}

BoundaryLine& BoundaryLine::setG(double g) {
    backend_->setDouble(handle_, prop::BL_G, g);
    return *this;
}

double BoundaryLine::getB() const {
    return backend_->getDouble(handle_, prop::BL_B);
}

BoundaryLine& BoundaryLine::setB(double b) {
    backend_->setDouble(handle_, prop::BL_B, b);
    return *this;
}

bool BoundaryLine::isPaired() const {
    return backend_->getBool(handle_, prop::BL_PAIRED);
}

iidm::optional<std::string> BoundaryLine::getPairingKey() const {
    return backend_->getOptString(handle_, prop::BL_PAIRING_KEY);
}

BoundaryLine& BoundaryLine::setPairingKey(const std::string& pairingKey) {
    backend_->setString(handle_, prop::BL_PAIRING_KEY, pairingKey);
    return *this;
}

iidm::optional<BoundaryLine::Generation> BoundaryLine::getGeneration() const {
    return detail::wrapOptional<Generation>(backend_->getRelated(handle_, Generation::TYPE), backend_);
}

Boundary BoundaryLine::getBoundary() const {
    return Boundary(backend_->getRelated(handle_, Boundary::TYPE), backend_);
}

// ── Solved values ────────────────────────────────────────────────────────────

void BoundaryLine::applySolvedValues() {
    backend_->invoke(handle_, prop::ACT_APPLY_SOLVED_VALUES);
}

void BoundaryLine::setGenerationTargetPToP() {
    backend_->invoke(handle_, prop::ACT_SET_TARGET_P_TO_P);
}

void BoundaryLine::setGenerationTargetQToQ() {
    backend_->invoke(handle_, prop::ACT_SET_TARGET_Q_TO_Q);
}

void BoundaryLine::setGenerationTargetVToV() {
    backend_->invoke(handle_, prop::ACT_SET_TARGET_V_TO_V);
}

// ── Generation ───────────────────────────────────────────────────────────────

double BoundaryLine::Generation::getTargetP() const {
    return backend_->getDouble(handle_, prop::GENERATION_TARGET_P);
}

BoundaryLine::Generation& BoundaryLine::Generation::setTargetP(double targetP) {
    backend_->setDouble(handle_, prop::GENERATION_TARGET_P, targetP);
    return *this;
}

double BoundaryLine::Generation::getMinP() const {
    return backend_->getDouble(handle_, prop::GENERATION_MIN_P);
}

BoundaryLine::Generation& BoundaryLine::Generation::setMinP(double minP) {
    backend_->setDouble(handle_, prop::GENERATION_MIN_P, minP);
    return *this;
}

double BoundaryLine::Generation::getMaxP() const {
    return backend_->getDouble(handle_, prop::GENERATION_MAX_P);
}

BoundaryLine::Generation& BoundaryLine::Generation::setMaxP(double maxP) {
    backend_->setDouble(handle_, prop::GENERATION_MAX_P, maxP);
    return *this;
}

double BoundaryLine::Generation::getTargetQ() const {
    return backend_->getDouble(handle_, prop::GENERATION_TARGET_Q);
}

BoundaryLine::Generation& BoundaryLine::Generation::setTargetQ(double targetQ) {
    backend_->setDouble(handle_, prop::GENERATION_TARGET_Q, targetQ);
    return *this;
}

bool BoundaryLine::Generation::isVoltageRegulationOn() const {
    return backend_->getBool(handle_, prop::GENERATION_VOLTAGE_REGULATION_ON);
}

BoundaryLine::Generation& BoundaryLine::Generation::setVoltageRegulationOn(bool voltageRegulationOn) {
    backend_->setBool(handle_, prop::GENERATION_VOLTAGE_REGULATION_ON, voltageRegulationOn);
    return *this;
}

double BoundaryLine::Generation::getTargetV() const {
    return backend_->getDouble(handle_, prop::GENERATION_TARGET_V);
}

BoundaryLine::Generation& BoundaryLine::Generation::setTargetV(double targetV) {
    backend_->setDouble(handle_, prop::GENERATION_TARGET_V, targetV);
    return *this;
}

// ── Boundary ─────────────────────────────────────────────────────────────────

double Boundary::getV() const {
    return backend_->getDouble(handle_, prop::BOUNDARY_V);
}

double Boundary::getAngle() const {
    return backend_->getDouble(handle_, prop::BOUNDARY_ANGLE);
}

double Boundary::getP() const {
    return backend_->getDouble(handle_, prop::BOUNDARY_P);
}

double Boundary::getQ() const {
    return backend_->getDouble(handle_, prop::BOUNDARY_Q);
}

double Boundary::getI() const {
    return backend_->getDouble(handle_, prop::BOUNDARY_I);
}

BoundaryLine Boundary::getBoundaryLine() const {
    return BoundaryLine(backend_->getRelated(handle_, BoundaryLine::TYPE), backend_);
}

VoltageLevel Boundary::getNetworkSideVoltageLevel() const {
    return VoltageLevel(backend_->getRelated(handle_, prop::REL_VOLTAGE_LEVEL), backend_);
}

std::vector<BoundaryLine> detail::filterBoundaryLines(std::vector<BoundaryLine> lines, BoundaryLineFilter filter) {
    if (filter == BoundaryLineFilter::ALL) return lines;
    const bool paired = filter == BoundaryLineFilter::PAIRED;
    lines.erase(std::remove_if(lines.begin(), lines.end(),
                               [paired](const BoundaryLine& line) { return line.isPaired() != paired; }),
                lines.end());
    return lines;
}

} // namespace iidm
