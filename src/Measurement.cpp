#include <iidm/Measurement.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <utility>

namespace iidm {

// ── Measurement ───────────────────────────────────────────────────────────────

Measurement::Measurement(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(detail::orNullBackend(backend)) {}

std::string Measurement::getId() const {
    return backend_->getString(handle_, prop::MEAS_ID);
}

MeasurementType Measurement::getType() const {
    return static_cast<MeasurementType>(backend_->getInt(handle_, prop::MEAS_TYPE));
}

iidm::optional<ThreeSides> Measurement::getSide() const {
    int side = backend_->getInt(handle_, prop::MEAS_SIDE);
    if (side < 0) return {};
    return static_cast<ThreeSides>(side);
}

double Measurement::getValue() const {
    return backend_->getDouble(handle_, prop::MEAS_VALUE);
}

Measurement& Measurement::setValue(double value) {
    backend_->setDouble(handle_, prop::MEAS_VALUE, value);
    return *this;
}

double Measurement::getStandardDeviation() const {
    return backend_->getDouble(handle_, prop::MEAS_STD_DEV);
}

Measurement& Measurement::setStandardDeviation(double standardDeviation) {
    backend_->setDouble(handle_, prop::MEAS_STD_DEV, standardDeviation);
    return *this;
}

bool Measurement::isValueValid() const {
    return backend_->getBool(handle_, prop::MEAS_VALID);
}

Measurement& Measurement::setValueValid(bool valid) {
    backend_->setBool(handle_, prop::MEAS_VALID, valid);
    return *this;
}

Measurement& Measurement::setValueAndValidity(double value, bool valid) {
    if (valid) {
        setValue(value);
        setValueValid(true);
    } else {
        setValueValid(false);
        setValue(value);
    }
    return *this;
}

bool Measurement::operator==(const Measurement& other) const {
    return handle_ == other.handle_;
}

bool Measurement::operator!=(const Measurement& other) const {
    return !(*this == other);
}

// ── Measurements ──────────────────────────────────────────────────────────────

Measurements::Measurements(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(detail::orNullBackend(backend)) {}

std::vector<Measurement> Measurements::getMeasurements() const {
    auto handles = backend_->getChildren(handle_, prop::MEASUREMENT);
    std::vector<Measurement> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<Measurement> Measurements::getMeasurements(MeasurementType type) const {
    std::vector<Measurement> result;
    for (auto& m : getMeasurements()) {
        if (m.getType() == type) result.push_back(std::move(m));
    }
    return result;
}

iidm::optional<Measurement> Measurements::getMeasurement(const std::string& id) const {
    ObjectHandle h = backend_->getRelatedByString(handle_, prop::REL_MEASUREMENT_BY_ID, id);
    if (h == INVALID_HANDLE) return {};
    return Measurement(h, backend_);
}

bool Measurements::operator==(const Measurements& other) const {
    return handle_ == other.handle_;
}

bool Measurements::operator!=(const Measurements& other) const {
    return !(*this == other);
}

} // namespace iidm
