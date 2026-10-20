#pragma once

#include <iidm/Bus.h>
#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl ObservabilityArea extension attached to a VoltageLevel
// (read-only: areas are produced by a state estimator).
class ObservabilityArea : public detail::ObjectRef {
public:
    // One observability area of the voltage level.
    class AreaCharacteristics : public detail::ObjectRef {
    public:
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

    std::vector<AreaCharacteristics> getObservabilityAreas() const;

    // Area containing a node (node-breaker voltage level only).
    iidm::optional<AreaCharacteristics> getObservabilityAreaByNode(int node) const;
    // Area containing a bus of the bus view.
    iidm::optional<AreaCharacteristics> getObservabilityAreaByBus(const std::string& busId) const;
    // Area containing a bus of the bus-breaker view.
    iidm::optional<AreaCharacteristics> getObservabilityAreaByBusBreakerBus(const std::string& busId) const;
};

} // namespace iidm
