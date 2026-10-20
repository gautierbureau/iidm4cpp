#pragma once

#include <iidm/Enums.h>
#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// One measurement of the PowSyBl DiscreteMeasurements extension (tap position, switch
// position, shunt section...).
//
// The validity flag of the measured value is exposed as isValueValid()/setValueValid():
// isValid() keeps the meaning it has on every iidm4cpp wrapper (the handle is valid).
class DiscreteMeasurement {
public:
    DiscreteMeasurement() = default;
    explicit DiscreteMeasurement(ObjectHandle handle, BackendProvider* backend);

    std::string             getId() const;
    DiscreteMeasurementType getType() const;

    // Measured tap changer for TAP_POSITION measurements; empty otherwise.
    iidm::optional<DiscreteMeasurementTapChanger> getTapChanger() const;

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

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const DiscreteMeasurement& other) const;
    bool operator!=(const DiscreteMeasurement& other) const;

private:
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

// Typed wrapper for the PowSyBl DiscreteMeasurements extension attached to an identifiable
// (Switch, TwoWindingsTransformer, ThreeWindingsTransformer, ShuntCompensator).
class DiscreteMeasurements {
public:
    DiscreteMeasurements() = default;
    explicit DiscreteMeasurements(ObjectHandle handle, BackendProvider* backend);

    std::vector<DiscreteMeasurement> getDiscreteMeasurements() const;
    std::vector<DiscreteMeasurement> getDiscreteMeasurements(DiscreteMeasurementType type) const;
    iidm::optional<DiscreteMeasurement> getDiscreteMeasurement(const std::string& id) const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const DiscreteMeasurements& other) const;
    bool operator!=(const DiscreteMeasurements& other) const;

private:
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
