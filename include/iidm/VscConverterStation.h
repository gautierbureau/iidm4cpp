#pragma once

#include <iidm/HvdcConverterStation.h>
#include <iidm/ReactiveLimitsHolder.h>
#include <iidm/VoltageRegulationHolder.h>

namespace iidm {

class VscConverterStation : public HvdcConverterStation,
                            public detail::VoltageRegulationHolderMethods<VscConverterStation>,
                            public detail::ReactiveLimitsHolderMethods<VscConverterStation> {
public:
    static constexpr int TYPE = prop::VSC_CONVERTER_STATION;

    VscConverterStation() = default;
    explicit VscConverterStation(ObjectHandle handle, BackendProvider* backend) : HvdcConverterStation(handle, backend) {}
};

} // namespace iidm
