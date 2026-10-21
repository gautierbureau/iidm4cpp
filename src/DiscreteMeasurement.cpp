#include <iidm/DiscreteMeasurement.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"
#include <utility>

namespace iidm {

// ── DiscreteMeasurement ───────────────────────────────────────────────────────

DiscreteMeasurement::DiscreteMeasurement(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

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

DiscreteMeasurement& DiscreteMeasurement::setValue(const char* value) {
    return setValue(std::string(value));
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
