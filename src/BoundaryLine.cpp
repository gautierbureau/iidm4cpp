#include <iidm/BoundaryLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/TieLine.h>
#include <iidm/VoltageLevel.h>
#include "BoundaryLineFilter.h"
#include "Wrap.h"

#include <algorithm>

namespace iidm {

iidm::optional<TieLine> BoundaryLine::getTieLine() const {
    return detail::wrapOptional<TieLine>(backend_->getRelated(handle_, prop::TIE_LINE), backend_);
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

// ── Boundary ─────────────────────────────────────────────────────────────────

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
