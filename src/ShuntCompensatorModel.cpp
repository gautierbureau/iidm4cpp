#include <iidm/ShuntCompensatorModel.h>
#include <iidm/BackendProvider.h>
#include <iidm/ShuntCompensatorLinearModel.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include "Wrap.h"

namespace iidm {

// ── ShuntCompensatorLinearModel ──────────────────────────────────────────────

double ShuntCompensatorLinearModel::getBPerSection() const {
    return backend_->getDouble(handle_, prop::SHUNT_B_PER_SECTION);
}

ShuntCompensatorLinearModel& ShuntCompensatorLinearModel::setBPerSection(double bPerSection) {
    backend_->setDouble(handle_, prop::SHUNT_B_PER_SECTION, bPerSection);
    return *this;
}

double ShuntCompensatorLinearModel::getGPerSection() const {
    return backend_->getDouble(handle_, prop::SHUNT_G_PER_SECTION);
}

ShuntCompensatorLinearModel& ShuntCompensatorLinearModel::setGPerSection(double gPerSection) {
    backend_->setDouble(handle_, prop::SHUNT_G_PER_SECTION, gPerSection);
    return *this;
}

ShuntCompensatorLinearModel& ShuntCompensatorLinearModel::setMaximumSectionCount(int maximumSectionCount) {
    backend_->setInt(handle_, prop::SHUNT_MAX_SECTION_COUNT, maximumSectionCount);
    return *this;
}

// ── ShuntCompensatorNonLinearModel ───────────────────────────────────────────

double ShuntCompensatorNonLinearModel::Section::getB() const {
    return backend_->getDouble(handle_, prop::SHUNT_SECTION_B);
}

ShuntCompensatorNonLinearModel::Section& ShuntCompensatorNonLinearModel::Section::setB(double b) {
    backend_->setDouble(handle_, prop::SHUNT_SECTION_B, b);
    return *this;
}

double ShuntCompensatorNonLinearModel::Section::getG() const {
    return backend_->getDouble(handle_, prop::SHUNT_SECTION_G);
}

ShuntCompensatorNonLinearModel::Section& ShuntCompensatorNonLinearModel::Section::setG(double g) {
    backend_->setDouble(handle_, prop::SHUNT_SECTION_G, g);
    return *this;
}

std::vector<ShuntCompensatorNonLinearModel::Section> ShuntCompensatorNonLinearModel::getAllSections() const {
    return detail::wrapAll<Section>(backend_->getChildren(handle_, Section::TYPE), backend_);
}

} // namespace iidm
