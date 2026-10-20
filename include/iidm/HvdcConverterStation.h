#pragma once

#include <iidm/Enums.h>
#include <iidm/Injection.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class HvdcLine;

// A converter station at either end of an HVDC line (powsybl HvdcConverterStation):
// a VscConverterStation or an LccConverterStation.
class HvdcConverterStation : public Injection {
public:
    static constexpr int TYPE = prop::HVDC_CONVERTER_STATION;

    HvdcConverterStation() = default;
    explicit HvdcConverterStation(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    HvdcType getHvdcType() const;

    double getLossFactor() const;
    HvdcConverterStation& setLossFactor(double lossFactor);

    // Empty when the station is not in an HVDC line
    iidm::optional<HvdcLine> getHvdcLine() const;
    iidm::optional<HvdcConverterStation> getOtherConverterStation() const;
};

} // namespace iidm
