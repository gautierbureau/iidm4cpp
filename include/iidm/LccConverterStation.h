#pragma once

#include <iidm/HvdcConverterStation.h>

namespace iidm {

class LccConverterStation : public HvdcConverterStation {
public:
    static constexpr int TYPE = prop::LCC_CONVERTER_STATION;

    LccConverterStation() = default;
    explicit LccConverterStation(ObjectHandle handle, BackendProvider* backend) : HvdcConverterStation(handle, backend) {}

    double getPowerFactor() const { return getDouble(prop::LCC_POWER_FACTOR); }
    LccConverterStation& setPowerFactor(double powerFactor) {
        return setDouble(*this, prop::LCC_POWER_FACTOR, powerFactor);
    }
};

} // namespace iidm
