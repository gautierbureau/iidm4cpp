#include <iidm/ShuntCompensator.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

// ── Section counts ───────────────────────────────────────────────────────────

ShuntCompensator& ShuntCompensator::unsetSectionCount() {
    backend_->invoke(handle_, prop::ACT_UNSET_SECTION_COUNT);
    return *this;
}

iidm::optional<int> ShuntCompensator::findSolvedSectionCount() const {
    return getSolvedSectionCount();
}

ShuntCompensator& ShuntCompensator::unsetSolvedSectionCount() {
    backend_->invoke(handle_, prop::ACT_UNSET_SOLVED_SECTION_COUNT);
    return *this;
}

// ── Susceptance and conductance ──────────────────────────────────────────────

double ShuntCompensator::getB(int sectionCount) const {
    return backend_->evaluate(handle_, prop::SHUNT_B, {static_cast<double>(sectionCount)});
}

double ShuntCompensator::getG(int sectionCount) const {
    return backend_->evaluate(handle_, prop::SHUNT_G, {static_cast<double>(sectionCount)});
}

// ── Model ────────────────────────────────────────────────────────────────────

ShuntCompensatorModel ShuntCompensator::getModel() const {
    return ShuntCompensatorModel(backend_->getRelated(handle_, ShuntCompensatorModel::TYPE), backend_);
}

// ── Solved values ────────────────────────────────────────────────────────────

void ShuntCompensator::applySolvedValues() {
    backend_->invoke(handle_, prop::ACT_APPLY_SOLVED_VALUES);
}

void ShuntCompensator::setSectionCountToSolvedSectionCount() {
    backend_->invoke(handle_, prop::ACT_SET_SECTION_COUNT_TO_SOLVED_SECTION_COUNT);
}

} // namespace iidm
