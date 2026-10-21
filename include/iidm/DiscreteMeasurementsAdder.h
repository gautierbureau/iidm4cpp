#pragma once

#include <iidm/Adder.h>
#include <iidm/DiscreteMeasurement.h>

namespace iidm {

// New DiscreteMeasurements extension of an identifiable, without measurements: they are added with
// DiscreteMeasurements::newDiscreteMeasurement()
class DiscreteMeasurementsAdder : public detail::AdderBase<DiscreteMeasurementsAdder> {
public:
    DiscreteMeasurementsAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    DiscreteMeasurements add() { return DiscreteMeasurements(create(), backend_); }
};

} // namespace iidm
