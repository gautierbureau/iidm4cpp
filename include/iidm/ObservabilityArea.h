#pragma once

#include <iidm/Bus.h>
#include <iidm/Enums.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl ObservabilityArea extension attached to a VoltageLevel
// (read-only: areas are produced by a state estimator).
class ObservabilityArea {
public:
    // One observability area of the voltage level.
    class AreaCharacteristics {
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

        bool isValid() const { return handle_ != INVALID_HANDLE; }

    private:
        ObjectHandle     handle_  = INVALID_HANDLE;
        BackendProvider* backend_ = nullptr;
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

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ObservabilityArea& other) const;
    bool operator!=(const ObservabilityArea& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
