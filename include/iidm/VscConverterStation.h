#pragma once

#include <iidm/HvdcConverterStation.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/VoltageRegulationHolder.h>

namespace iidm {

class VscConverterStation : public HvdcConverterStation, public detail::VoltageRegulationHolderMethods<VscConverterStation> {
public:
    static constexpr int TYPE = prop::VSC_CONVERTER_STATION;

    VscConverterStation() = default;
    explicit VscConverterStation(ObjectHandle handle, BackendProvider* backend) : HvdcConverterStation(handle, backend) {}

    // ── Reactive limits ──────────────────────────────────────────────────
    bool hasMinMaxReactiveLimits() const;
    MinMaxReactiveLimits getMinMaxReactiveLimits() const;

    bool hasReactiveCapabilityCurve() const;
    ReactiveCapabilityCurve getReactiveCapabilityCurve() const;
};

} // namespace iidm
