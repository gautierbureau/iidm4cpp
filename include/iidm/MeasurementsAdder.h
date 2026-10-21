#pragma once

#include <iidm/Adder.h>
#include <iidm/Measurement.h>

namespace iidm {

// New Measurements extension of a connectable, without measurements: they are added with
// Measurements::newMeasurement()
class MeasurementsAdder : public detail::AdderBase<MeasurementsAdder> {
public:
    MeasurementsAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    Measurements add() { return Measurements(create(), backend_); }
};

} // namespace iidm
