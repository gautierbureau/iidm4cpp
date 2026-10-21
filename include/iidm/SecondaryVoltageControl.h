#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// The pilot point of a control zone: the busbar sections or buses whose voltage the zone controls
class PilotPoint : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::PILOT_POINT;

    PilotPoint() = default;
    explicit PilotPoint(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    // Ids of busbar sections or of buses of the bus-breaker view, in order
    std::vector<std::string> getBusbarSectionsOrBusesIds() const;
    // kV, for the working variant
    double getTargetV() const { return getDouble(prop::EXT_SVC_TARGET_V); }
    PilotPoint& setTargetV(double targetV) { return setDouble(*this, prop::EXT_SVC_TARGET_V, targetV); }
};

// A control unit of a control zone: an equipment that controls the voltage of the pilot point
class ControlUnit : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::CONTROL_UNIT;

    ControlUnit() = default;
    explicit ControlUnit(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    // The id of its equipment
    std::string getId() const { return getString(prop::EXT_SVC_UNIT_ID); }
    // For the working variant
    bool isParticipate() const { return getBool(prop::EXT_SVC_UNIT_PARTICIPATE); }
    ControlUnit& setParticipate(bool participate) {
        return setBool(*this, prop::EXT_SVC_UNIT_PARTICIPATE, participate);
    }
};

// A control zone of the SecondaryVoltageControl extension: its pilot point and its control units
class ControlZone : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::CONTROL_ZONE;

    ControlZone() = default;
    explicit ControlZone(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    std::string getName() const { return getString(prop::EXT_SVC_ZONE_NAME); }
    PilotPoint getPilotPoint() const;
    // In order
    std::vector<ControlUnit> getControlUnits() const;
    // Empty when the zone has none with that id
    iidm::optional<ControlUnit> getControlUnit(const std::string& id) const;
};

// The SecondaryVoltageControl extension of a network: its control zones. An object of its own, from
// Network::getExtension<SecondaryVoltageControl>(); its zones, pilot points and control units are
// given at creation (Network::newExtension<SecondaryVoltageControlAdder>()).
class SecondaryVoltageControl : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::SECONDARY_VOLTAGE_CONTROL;

    SecondaryVoltageControl() = default;
    explicit SecondaryVoltageControl(ObjectHandle handle, BackendProvider* backend);

    // In order
    std::vector<ControlZone> getControlZones() const;
    // Empty when there is none with that name
    iidm::optional<ControlZone> getControlZone(const std::string& name) const;
};

} // namespace iidm
