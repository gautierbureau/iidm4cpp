#include <iidm/ShuntCompensator.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

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

iidm::optional<int> ShuntCompensator::getSolvedSectionCount() const {
    return backend_->getOptInt(handle_, prop::SHUNT_SOLVED_SECTION_COUNT);
}

ShuntCompensator& ShuntCompensator::setSolvedSectionCount(int count) {
    backend_->setInt(handle_, prop::SHUNT_SOLVED_SECTION_COUNT, count);
    return *this;
}

int ShuntCompensator::getMaximumSectionCount() const {
    return backend_->getInt(handle_, prop::SHUNT_MAX_SECTION_COUNT);
}

bool ShuntCompensator::hasLinearModel() const {
    return backend_->getInt(handle_, prop::SHUNT_MODEL_KIND) == 0;
}

double ShuntCompensator::getBPerSection() const {
    return backend_->getDouble(handle_, prop::SHUNT_B_PER_SECTION);
}

double ShuntCompensator::getGPerSection() const {
    return backend_->getDouble(handle_, prop::SHUNT_G_PER_SECTION);
}

double ShuntCompensator::getB() const {
    return backend_->getDouble(handle_, prop::SHUNT_B);
}

bool ShuntCompensator::hasNonLinearModel() const {
    return backend_->getInt(handle_, prop::SHUNT_MODEL_KIND) == 1;
}

ShuntCompensatorNonLinearModel ShuntCompensator::getNonLinearModel() const {
    return ShuntCompensatorNonLinearModel(handle_, backend_);
}

} // namespace iidm
