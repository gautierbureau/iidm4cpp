#pragma once

#include <iidm/Bus.h>
#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// The ObservabilityArea extension of a voltage level: an object of its own, from
// VoltageLevel::getObservabilityArea() (read-only: its areas come from a state estimator).
class ObservabilityArea : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::OBSERVABILITY_AREA;

    // One observability area of the voltage level.
    class AreaCharacteristics : public detail::ObjectRef {
    public:
        static constexpr int TYPE = prop::AREA_CHARACTERISTICS;

        AreaCharacteristics() = default;
        explicit AreaCharacteristics(ObjectHandle handle, BackendProvider* backend);

        int                 getAreaNumber() const;
        ObservabilityStatus getStatus() const;

        // Terminals of the voltage level that belong to the area.
        std::vector<Terminal> getTerminals() const;
        // Node-breaker voltage level: the area's nodes (sorted); empty in bus-breaker topology.
        std::vector<int> getNodes() const;
        // Bus-breaker voltage level: the area's bus-breaker view buses; empty in node-breaker topology.
        std::vector<Bus> getBuses() const;
    };

    ObservabilityArea() = default;
    explicit ObservabilityArea(ObjectHandle handle, BackendProvider* backend);

    bool isConsistentWithTopology() const;

    // By area number
    std::vector<AreaCharacteristics> getObservabilityAreas() const;

    // Area containing a node (node-breaker voltage level only).
    iidm::optional<AreaCharacteristics> getObservabilityAreaByNode(int node) const;
    // Area containing a bus of the bus view.
    iidm::optional<AreaCharacteristics> getObservabilityAreaByBus(const std::string& busId) const;
    // Area containing a bus of the bus-breaker view.
    iidm::optional<AreaCharacteristics> getObservabilityAreaByBusBreakerBus(const std::string& busId) const;
};

} // namespace iidm
