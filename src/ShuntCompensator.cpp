#include <iidm/ShuntCompensator.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ShuntCompensator::ShuntCompensator(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string ShuntCompensator::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string ShuntCompensator::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

int ShuntCompensator::getSectionCount() const {
    return backend_->getInt(handle_, prop::SHUNT_SECTION_COUNT);
}

ShuntCompensator& ShuntCompensator::setSectionCount(int count) {
    backend_->setInt(handle_, prop::SHUNT_SECTION_COUNT, count);
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

bool ShuntCompensator::hasNonLinearModel() const {
    return backend_->getInt(handle_, prop::SHUNT_MODEL_KIND) == 1;
}

ShuntCompensatorNonLinearModel ShuntCompensator::getNonLinearModel() const {
    return ShuntCompensatorNonLinearModel(handle_, backend_);
}

Terminal ShuntCompensator::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void ShuntCompensator::connect() {
    getTerminal().connect();
}

void ShuntCompensator::disconnect() {
    getTerminal().disconnect();
}

bool ShuntCompensator::isConnected() const {
    return getTerminal().isConnected();
}

bool ShuntCompensator::operator==(const ShuntCompensator& other) const {
    return handle_ == other.handle_;
}

bool ShuntCompensator::operator!=(const ShuntCompensator& other) const {
    return !(*this == other);
}

} // namespace iidm
