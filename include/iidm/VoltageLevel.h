#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/SlackTerminal.h>
#include <iidm/BusbarSection.h>
#include <iidm/Switch.h>
#include <optional>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

class VoltageLevel {
public:
    VoltageLevel() = default;
    explicit VoltageLevel(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getNominalV() const;
    std::optional<double> getLowVoltageLimit() const;
    std::optional<double> getHighVoltageLimit() const;

    TopologyKind getTopologyKind() const;

    bool hasSlackTerminal() const;
    SlackTerminal getSlackTerminal() const;

    std::vector<Switch> getSwitches() const;
    std::vector<BusbarSection> getBusbarSections() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const VoltageLevel& other) const;
    bool operator!=(const VoltageLevel& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
