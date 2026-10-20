#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// One measurement of the PowSyBl Measurements extension.
//
// The validity flag of the measured value is exposed as isValueValid()/setValueValid():
// isValid() keeps the meaning it has on every iidm4cpp wrapper (the handle is valid).
class Measurement : public detail::ObjectRef {
public:
    Measurement() = default;
    explicit Measurement(ObjectHandle handle, BackendProvider* backend);

    std::string     getId() const;
    MeasurementType getType() const;

    // Side of the measured branch / three-windings transformer; empty for injections.
    iidm::optional<ThreeSides> getSide() const;

    double getValue() const;
    Measurement& setValue(double value);

    double getStandardDeviation() const;
    Measurement& setStandardDeviation(double standardDeviation);

    bool isValueValid() const;
    Measurement& setValueValid(bool valid);

    // Same as powsybl-core's setValueAndValidity: the validity is never true for a stale value.
    Measurement& setValueAndValidity(double value, bool valid);
};

// Typed wrapper for the PowSyBl Measurements extension attached to a connectable.
class Measurements : public detail::ObjectRef {
public:
    Measurements() = default;
    explicit Measurements(ObjectHandle handle, BackendProvider* backend);

    std::vector<Measurement> getMeasurements() const;
    std::vector<Measurement> getMeasurements(MeasurementType type) const;
    iidm::optional<Measurement> getMeasurement(const std::string& id) const;
};

} // namespace iidm
