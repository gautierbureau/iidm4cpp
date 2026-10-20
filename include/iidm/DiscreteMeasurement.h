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

// One measurement of a DiscreteMeasurements extension (tap position, switch position, shunt
// section...).
//
// The validity flag of the measured value is exposed as isValueValid()/setValueValid():
// isValid() keeps the meaning it has on every iidm4cpp wrapper (the handle is valid).
class DiscreteMeasurement : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::DISCRETE_MEASUREMENT;

    DiscreteMeasurement() = default;
    explicit DiscreteMeasurement(ObjectHandle handle, BackendProvider* backend);

    // Empty when it has none
    iidm::optional<std::string> getId() const;
    DiscreteMeasurementType getType() const;

    // Measured tap changer of a TAP_POSITION measurement; empty otherwise.
    iidm::optional<DiscreteMeasurementTapChanger> getTapChanger() const;

    // Sorted
    std::vector<std::string> getPropertyNames() const;
    // Empty when there is no such property
    iidm::optional<std::string> getProperty(const std::string& name) const;
    DiscreteMeasurement& putProperty(const std::string& name, const std::string& value);
    DiscreteMeasurement& removeProperty(const std::string& name);

    DiscreteMeasurementValueType getValueType() const;

    // Each getter requires the matching value type.
    std::string getValueAsString() const;
    int         getValueAsInt() const;
    bool        getValueAsBoolean() const;

    DiscreteMeasurement& setValue(const std::string& value);
    DiscreteMeasurement& setValue(const char* value);   // avoids the const char* → bool conversion
    DiscreteMeasurement& setValue(int value);
    DiscreteMeasurement& setValue(bool value);

    bool isValueValid() const;
    DiscreteMeasurement& setValueValid(bool valid);

    // Removes it from its extension
    void remove();
};

// New measurement of a DiscreteMeasurements extension
// (DiscreteMeasurements::newDiscreteMeasurement()). Its value type is the type of its value.
class DiscreteMeasurementAdder : public detail::AdderBase<DiscreteMeasurementAdder> {
public:
    DiscreteMeasurementAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    // Optional
    DiscreteMeasurementAdder& setId(const std::string& id) { return stringField(prop::DMEAS_ID, id); }
    // A free id is derived from the given one if it is already taken
    DiscreteMeasurementAdder& setEnsureIdUnicity(bool ensureIdUnicity) { return boolField(prop::ENSURE_ID_UNICITY, ensureIdUnicity); }
    DiscreteMeasurementAdder& putProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }
    DiscreteMeasurementAdder& setType(DiscreteMeasurementType type) { return intField(prop::DMEAS_TYPE, static_cast<int>(type)); }
    DiscreteMeasurementAdder& setTapChanger(DiscreteMeasurementTapChanger tapChanger) {
        return intField(prop::DMEAS_TAP_CHANGER, static_cast<int>(tapChanger));
    }
    DiscreteMeasurementAdder& setValue(const std::string& value) { return stringField(prop::DMEAS_VALUE_STRING, value); }
    DiscreteMeasurementAdder& setValue(const char* value)        { return stringField(prop::DMEAS_VALUE_STRING, value); }
    DiscreteMeasurementAdder& setValue(int value)                { return intField(prop::DMEAS_VALUE_INT, value); }
    DiscreteMeasurementAdder& setValue(bool value)               { return boolField(prop::DMEAS_VALUE_BOOL, value); }
    DiscreteMeasurementAdder& setValid(bool valid)               { return boolField(prop::DMEAS_VALID, valid); }

    DiscreteMeasurement add() { return DiscreteMeasurement(create(), backend_); }
};

// The DiscreteMeasurements extension of an identifiable (switches, transformers, shunts...): an
// object of its own, from Identifiable::getDiscreteMeasurements().
class DiscreteMeasurements : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::DISCRETE_MEASUREMENTS;

    DiscreteMeasurements() = default;
    explicit DiscreteMeasurements(ObjectHandle handle, BackendProvider* backend);

    std::vector<DiscreteMeasurement> getDiscreteMeasurements() const;
    std::vector<DiscreteMeasurement> getDiscreteMeasurements(DiscreteMeasurementType type) const;
    // Empty when there is no such measurement
    iidm::optional<DiscreteMeasurement> getDiscreteMeasurement(const std::string& id) const;

    DiscreteMeasurementAdder newDiscreteMeasurement();
    // Removes the extension from its identifiable when it has no measurement
    void cleanIfEmpty();
};

} // namespace iidm
