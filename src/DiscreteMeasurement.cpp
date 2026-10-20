#include <iidm/DiscreteMeasurement.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"
#include <utility>

namespace iidm {

// ── DiscreteMeasurement ───────────────────────────────────────────────────────

DiscreteMeasurement::DiscreteMeasurement(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

iidm::optional<std::string> DiscreteMeasurement::getId() const {
    return backend_->getOptString(handle_, prop::DMEAS_ID);
}

DiscreteMeasurementType DiscreteMeasurement::getType() const {
    return static_cast<DiscreteMeasurementType>(backend_->getInt(handle_, prop::DMEAS_TYPE));
}

iidm::optional<DiscreteMeasurementTapChanger> DiscreteMeasurement::getTapChanger() const {
    int tapChanger = backend_->getInt(handle_, prop::DMEAS_TAP_CHANGER);
    if (tapChanger < 0) return {};
    return static_cast<DiscreteMeasurementTapChanger>(tapChanger);
}

std::vector<std::string> DiscreteMeasurement::getPropertyNames() const {
    return backend_->getStringList(handle_, prop::PROPERTY_NAMES);
}

iidm::optional<std::string> DiscreteMeasurement::getProperty(const std::string& name) const {
    return backend_->getKeyedString(handle_, prop::PROPERTY, name);
}

DiscreteMeasurement& DiscreteMeasurement::putProperty(const std::string& name, const std::string& value) {
    backend_->setKeyedString(handle_, prop::PROPERTY, name, value);
    return *this;
}

DiscreteMeasurement& DiscreteMeasurement::removeProperty(const std::string& name) {
    backend_->setKeyedString(handle_, prop::PROPERTY, name, iidm::optional<std::string>());
    return *this;
}

DiscreteMeasurementValueType DiscreteMeasurement::getValueType() const {
    return static_cast<DiscreteMeasurementValueType>(backend_->getInt(handle_, prop::DMEAS_VALUE_TYPE));
}

std::string DiscreteMeasurement::getValueAsString() const {
    return backend_->getString(handle_, prop::DMEAS_VALUE_STRING);
}

int DiscreteMeasurement::getValueAsInt() const {
    return backend_->getInt(handle_, prop::DMEAS_VALUE_INT);
}

bool DiscreteMeasurement::getValueAsBoolean() const {
    return backend_->getBool(handle_, prop::DMEAS_VALUE_BOOL);
}

DiscreteMeasurement& DiscreteMeasurement::setValue(const std::string& value) {
    backend_->setString(handle_, prop::DMEAS_VALUE_STRING, value);
    return *this;
}

DiscreteMeasurement& DiscreteMeasurement::setValue(const char* value) {
    return setValue(std::string(value));
}

DiscreteMeasurement& DiscreteMeasurement::setValue(int value) {
    backend_->setInt(handle_, prop::DMEAS_VALUE_INT, value);
    return *this;
}

DiscreteMeasurement& DiscreteMeasurement::setValue(bool value) {
    backend_->setBool(handle_, prop::DMEAS_VALUE_BOOL, value);
    return *this;
}

bool DiscreteMeasurement::isValueValid() const {
    return backend_->getBool(handle_, prop::DMEAS_VALID);
}

DiscreteMeasurement& DiscreteMeasurement::setValueValid(bool valid) {
    backend_->setBool(handle_, prop::DMEAS_VALID, valid);
    return *this;
}

void DiscreteMeasurement::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

// ── DiscreteMeasurements ──────────────────────────────────────────────────────

DiscreteMeasurements::DiscreteMeasurements(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::vector<DiscreteMeasurement> DiscreteMeasurements::getDiscreteMeasurements() const {
    return detail::wrapAll<DiscreteMeasurement>(backend_->getChildren(handle_, prop::DISCRETE_MEASUREMENT), backend_);
}

std::vector<DiscreteMeasurement> DiscreteMeasurements::getDiscreteMeasurements(DiscreteMeasurementType type) const {
    std::vector<DiscreteMeasurement> result;
    for (DiscreteMeasurement& m : getDiscreteMeasurements()) {
        if (m.getType() == type) result.push_back(std::move(m));
    }
    return result;
}

iidm::optional<DiscreteMeasurement> DiscreteMeasurements::getDiscreteMeasurement(const std::string& id) const {
    return detail::wrapOptional<DiscreteMeasurement>(
        backend_->getRelatedByString(handle_, prop::DISCRETE_MEASUREMENT, id), backend_);
}

DiscreteMeasurementAdder DiscreteMeasurements::newDiscreteMeasurement() {
    return DiscreteMeasurementAdder(backend_->invoke(handle_, prop::DISCRETE_MEASUREMENT), backend_);
}

void DiscreteMeasurements::cleanIfEmpty() {
    backend_->invoke(handle_, prop::ACT_CLEAN_IF_EMPTY);
}

} // namespace iidm
