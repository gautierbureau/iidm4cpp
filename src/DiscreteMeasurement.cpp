#include <iidm/DiscreteMeasurement.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <utility>

namespace iidm {

// ── DiscreteMeasurement ───────────────────────────────────────────────────────

DiscreteMeasurement::DiscreteMeasurement(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle, backend), backend_(detail::orNullBackend(backend)) {}

std::string DiscreteMeasurement::getId() const {
    return backend_->getString(handle_, prop::DMEAS_ID);
}

DiscreteMeasurementType DiscreteMeasurement::getType() const {
    return static_cast<DiscreteMeasurementType>(backend_->getInt(handle_, prop::DMEAS_TYPE));
}

iidm::optional<DiscreteMeasurementTapChanger> DiscreteMeasurement::getTapChanger() const {
    int tapChanger = backend_->getInt(handle_, prop::DMEAS_TAP_CHANGER);
    if (tapChanger < 0) return {};
    return static_cast<DiscreteMeasurementTapChanger>(tapChanger);
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

bool DiscreteMeasurement::operator==(const DiscreteMeasurement& other) const {
    return handle_ == other.handle_;
}

bool DiscreteMeasurement::operator!=(const DiscreteMeasurement& other) const {
    return !(*this == other);
}

// ── DiscreteMeasurements ──────────────────────────────────────────────────────

DiscreteMeasurements::DiscreteMeasurements(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle, backend), backend_(detail::orNullBackend(backend)) {}

std::vector<DiscreteMeasurement> DiscreteMeasurements::getDiscreteMeasurements() const {
    auto handles = backend_->getChildren(handle_, prop::DISCRETE_MEASUREMENT);
    std::vector<DiscreteMeasurement> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<DiscreteMeasurement> DiscreteMeasurements::getDiscreteMeasurements(DiscreteMeasurementType type) const {
    std::vector<DiscreteMeasurement> result;
    for (auto& m : getDiscreteMeasurements()) {
        if (m.getType() == type) result.push_back(std::move(m));
    }
    return result;
}

iidm::optional<DiscreteMeasurement> DiscreteMeasurements::getDiscreteMeasurement(const std::string& id) const {
    ObjectHandle h = backend_->getRelatedByString(handle_, prop::REL_DISCRETE_MEASUREMENT_BY_ID, id);
    if (h == INVALID_HANDLE) return {};
    return DiscreteMeasurement(h, backend_);
}

bool DiscreteMeasurements::operator==(const DiscreteMeasurements& other) const {
    return handle_ == other.handle_;
}

bool DiscreteMeasurements::operator!=(const DiscreteMeasurements& other) const {
    return !(*this == other);
}

} // namespace iidm
