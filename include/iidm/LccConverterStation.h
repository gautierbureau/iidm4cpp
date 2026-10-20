#pragma once

#include <iidm/HvdcConverterStation.h>

namespace iidm {

class LccConverterStation : public HvdcConverterStation {
public:
    static constexpr int TYPE = prop::LCC_CONVERTER_STATION;

    LccConverterStation() = default;
    explicit LccConverterStation(ObjectHandle handle, BackendProvider* backend) : HvdcConverterStation(handle, backend) {}

    double getPowerFactor() const;
};

} // namespace iidm
