#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// One measurement of a Measurements extension.
//
// The validity flag of the measured value is exposed as isValueValid()/setValueValid():
// isValid() keeps the meaning it has on every iidm4cpp wrapper (the handle is valid).
class Measurement : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::MEASUREMENT;

    Measurement() = default;
    explicit Measurement(ObjectHandle handle, BackendProvider* backend);

    // Empty when it has none
    iidm::optional<std::string> getId() const { return getOptString(prop::MEAS_ID); }
    MeasurementType getType() const { return static_cast<MeasurementType>(getInt(prop::MEAS_TYPE)); }

    // Sorted
    std::vector<std::string> getPropertyNames() const;
    // Empty when there is no such property
    iidm::optional<std::string> getProperty(const std::string& name) const;
    Measurement& putProperty(const std::string& name, const std::string& value);
    Measurement& removeProperty(const std::string& name);

    double getValue() const { return getDouble(prop::MEAS_VALUE); }
    Measurement& setValue(double value) { return setDouble(*this, prop::MEAS_VALUE, value); }

    double getStandardDeviation() const { return getDouble(prop::MEAS_STD_DEV); }
    Measurement& setStandardDeviation(double standardDeviation) {
        return setDouble(*this, prop::MEAS_STD_DEV, standardDeviation);
    }

    bool isValueValid() const { return getBool(prop::MEAS_VALID); }
    Measurement& setValueValid(bool valid) { return setBool(*this, prop::MEAS_VALID, valid); }

    // Same as powsybl's setValueAndValidity: the measurement is never valid with a stale value.
    Measurement& setValueAndValidity(double value, bool valid);

    // Side of the measured branch or three windings transformer; empty for an injection.
    iidm::optional<ThreeSides> getSide() const;

    // Removes it from its extension
    void remove();
};

// New measurement of a Measurements extension (Measurements::newMeasurement())
class MeasurementAdder : public detail::AdderBase<MeasurementAdder> {
public:
    MeasurementAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    // Optional
    MeasurementAdder& setId(const std::string& id) { return stringField(prop::MEAS_ID, id); }
    // A free id is derived from the given one if it is already taken
    MeasurementAdder& setEnsureIdUnicity(bool ensureIdUnicity) { return boolField(prop::ENSURE_ID_UNICITY, ensureIdUnicity); }
    MeasurementAdder& putProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }
    MeasurementAdder& setType(MeasurementType type)    { return intField(prop::MEAS_TYPE, static_cast<int>(type)); }
    MeasurementAdder& setValue(double value)           { return doubleField(prop::MEAS_VALUE, value); }
    MeasurementAdder& setStandardDeviation(double standardDeviation) { return doubleField(prop::MEAS_STD_DEV, standardDeviation); }
    MeasurementAdder& setSide(ThreeSides side)         { return intField(prop::MEAS_SIDE, static_cast<int>(side)); }
    MeasurementAdder& setValid(bool valid)             { return boolField(prop::MEAS_VALID, valid); }

    Measurement add() { return Measurement(create(), backend_); }
};

// The Measurements extension of a connectable: an object of its own, from
// Connectable::getMeasurements().
class Measurements : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::MEASUREMENTS;

    Measurements() = default;
    explicit Measurements(ObjectHandle handle, BackendProvider* backend);

    std::vector<Measurement> getMeasurements() const;
    std::vector<Measurement> getMeasurements(MeasurementType type) const;
    // Empty when there is no such measurement
    iidm::optional<Measurement> getMeasurement(const std::string& id) const;

    MeasurementAdder newMeasurement();
    // Removes the extension from its connectable when it has no measurement
    void cleanIfEmpty();
};

} // namespace iidm
