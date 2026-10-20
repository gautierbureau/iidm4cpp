#include <iidm/Measurement.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"
#include <utility>

namespace iidm {

// ── Measurement ───────────────────────────────────────────────────────────────

Measurement::Measurement(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

iidm::optional<std::string> Measurement::getId() const {
    return backend_->getOptString(handle_, prop::MEAS_ID);
}

MeasurementType Measurement::getType() const {
    return static_cast<MeasurementType>(backend_->getInt(handle_, prop::MEAS_TYPE));
}

std::vector<std::string> Measurement::getPropertyNames() const {
    return backend_->getStringList(handle_, prop::PROPERTY_NAMES);
}

iidm::optional<std::string> Measurement::getProperty(const std::string& name) const {
    return backend_->getKeyedString(handle_, prop::PROPERTY, name);
}

Measurement& Measurement::putProperty(const std::string& name, const std::string& value) {
    backend_->setKeyedString(handle_, prop::PROPERTY, name, value);
    return *this;
}

Measurement& Measurement::removeProperty(const std::string& name) {
    backend_->setKeyedString(handle_, prop::PROPERTY, name, iidm::optional<std::string>());
    return *this;
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

iidm::optional<ThreeSides> Measurement::getSide() const {
    int side = backend_->getInt(handle_, prop::MEAS_SIDE);
    if (side < 0) return {};
    return static_cast<ThreeSides>(side);
}

void Measurement::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

// ── Measurements ──────────────────────────────────────────────────────────────

Measurements::Measurements(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::vector<Measurement> Measurements::getMeasurements() const {
    return detail::wrapAll<Measurement>(backend_->getChildren(handle_, prop::MEASUREMENT), backend_);
}

std::vector<Measurement> Measurements::getMeasurements(MeasurementType type) const {
    std::vector<Measurement> result;
    for (Measurement& m : getMeasurements()) {
        if (m.getType() == type) result.push_back(std::move(m));
    }
    return result;
}

iidm::optional<Measurement> Measurements::getMeasurement(const std::string& id) const {
    return detail::wrapOptional<Measurement>(backend_->getRelatedByString(handle_, prop::MEASUREMENT, id), backend_);
}

MeasurementAdder Measurements::newMeasurement() {
    return MeasurementAdder(backend_->invoke(handle_, prop::MEASUREMENT), backend_);
}

void Measurements::cleanIfEmpty() {
    backend_->invoke(handle_, prop::ACT_CLEAN_IF_EMPTY);
}

} // namespace iidm
