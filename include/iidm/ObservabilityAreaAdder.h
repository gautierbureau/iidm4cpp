#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/HandleRef.h>
#include <iidm/ObservabilityArea.h>
#include <string>
#include <vector>

namespace iidm {

// New ObservabilityArea extension of a voltage level: its areas, each made of the buses or the
// nodes given
class ObservabilityAreaAdder : public detail::AdderBase<ObservabilityAreaAdder> {
public:
    ObservabilityAreaAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    // An area made of buses of the bus-breaker view (bus-breaker topology)
    ObservabilityAreaAdder& withObservabilityAreaByBusBreakerViewBuses(const std::vector<std::string>& busIds, int areaNumber,
                                                                       ObservabilityStatus status) {
        HandleRef area = newArea(areaNumber, status);
        for (const std::string& busId : busIds) backend_->setString(area, prop::OBS_AREA_BUS_BREAKER_BUS_ID, busId);
        backend_->invoke(area, prop::ACT_ADD);
        return *this;
    }

    // An area made of a bus of the bus view
    ObservabilityAreaAdder& withObservabilityAreaByBusViewBus(const std::string& busId, int areaNumber, ObservabilityStatus status) {
        HandleRef area = newArea(areaNumber, status);
        backend_->setString(area, prop::OBS_AREA_BUS_ID, busId);
        backend_->invoke(area, prop::ACT_ADD);
        return *this;
    }

    // An area made of nodes (node-breaker topology)
    ObservabilityAreaAdder& withObservabilityAreaByNodes(const std::vector<int>& nodes, int areaNumber, ObservabilityStatus status) {
        HandleRef area = newArea(areaNumber, status);
        for (int node : nodes) backend_->setInt(area, prop::OBS_AREA_NODE, node);
        backend_->invoke(area, prop::ACT_ADD);
        return *this;
    }

    ObservabilityArea add() { return ObservabilityArea(create(), backend_); }

private:
    HandleRef newArea(int areaNumber, ObservabilityStatus status) {
        HandleRef area(backend_->invoke(adder_, prop::AREA_CHARACTERISTICS), backend_);
        backend_->setInt(area, prop::OBS_AREA_NUMBER, areaNumber);
        backend_->setInt(area, prop::OBS_AREA_STATUS, static_cast<int>(status));
        return area;
    }
};

} // namespace iidm
