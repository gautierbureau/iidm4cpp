#include <iidm/ShuntCompensator.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

// ── Section counts ───────────────────────────────────────────────────────────

int ShuntCompensator::getSectionCount() const {
    return backend_->getInt(handle_, prop::SHUNT_SECTION_COUNT);
}

iidm::optional<int> ShuntCompensator::findSectionCount() const {
    return backend_->getOptInt(handle_, prop::SHUNT_SECTION_COUNT);
}

ShuntCompensator& ShuntCompensator::setSectionCount(int count) {
    backend_->setInt(handle_, prop::SHUNT_SECTION_COUNT, count);
    return *this;
}

ShuntCompensator& ShuntCompensator::unsetSectionCount() {
    backend_->invoke(handle_, prop::ACT_UNSET_SECTION_COUNT);
    return *this;
}

iidm::optional<int> ShuntCompensator::getSolvedSectionCount() const {
    return backend_->getOptInt(handle_, prop::SHUNT_SOLVED_SECTION_COUNT);
}

iidm::optional<int> ShuntCompensator::findSolvedSectionCount() const {
    return getSolvedSectionCount();
}

ShuntCompensator& ShuntCompensator::setSolvedSectionCount(int count) {
    backend_->setInt(handle_, prop::SHUNT_SOLVED_SECTION_COUNT, count);
    return *this;
}

ShuntCompensator& ShuntCompensator::unsetSolvedSectionCount() {
    backend_->invoke(handle_, prop::ACT_UNSET_SOLVED_SECTION_COUNT);
    return *this;
}

int ShuntCompensator::getMaximumSectionCount() const {
    return backend_->getInt(handle_, prop::SHUNT_MAX_SECTION_COUNT);
}

// ── Susceptance and conductance ──────────────────────────────────────────────

double ShuntCompensator::getB() const {
    return backend_->getDouble(handle_, prop::SHUNT_B);
}

double ShuntCompensator::getG() const {
    return backend_->getDouble(handle_, prop::SHUNT_G);
}

double ShuntCompensator::getB(int sectionCount) const {
    return backend_->evaluate(handle_, prop::SHUNT_B, {static_cast<double>(sectionCount)});
}

double ShuntCompensator::getG(int sectionCount) const {
    return backend_->evaluate(handle_, prop::SHUNT_G, {static_cast<double>(sectionCount)});
}

// ── Model ────────────────────────────────────────────────────────────────────

ShuntCompensatorModelType ShuntCompensator::getModelType() const {
    return static_cast<ShuntCompensatorModelType>(backend_->getInt(handle_, prop::SHUNT_MODEL_TYPE));
}

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
