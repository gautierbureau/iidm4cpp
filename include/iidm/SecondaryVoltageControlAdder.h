#pragma once

#include <iidm/Adder.h>
#include <iidm/SecondaryVoltageControl.h>
#include <string>
#include <vector>

namespace iidm {

class ControlZoneAdder;

// New SecondaryVoltageControl extension of a network (newSecondaryVoltageControl()): one control zone or
// more. ValidationException from add() without one.
class SecondaryVoltageControlAdder : public detail::AdderBase<SecondaryVoltageControlAdder> {
public:
    SecondaryVoltageControlAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    ControlZoneAdder newControlZone();

    SecondaryVoltageControl add() { return SecondaryVoltageControl(create(), backend_); }
};

class PilotPointAdder;
class ControlUnitAdder;

// A control zone of a new SecondaryVoltageControl: its name, its pilot point and one control unit or
// more (ValidationException from add() otherwise); add() adds it and returns to the extension's adder
class ControlZoneAdder : public detail::AdderBase<ControlZoneAdder> {
public:
    ControlZoneAdder(ObjectHandle adder, BackendProvider* backend, SecondaryVoltageControlAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    ControlZoneAdder& withName(const std::string& name) { return stringField(prop::EXT_SVC_ZONE_NAME, name); }
    PilotPointAdder newPilotPoint();
    ControlUnitAdder newControlUnit();

    SecondaryVoltageControlAdder& add() {
        create();
        return parent_;
    }

private:
    SecondaryVoltageControlAdder& parent_;
};

// The pilot point of a new control zone: one busbar section or bus or more, and a target voltage
// (ValidationException from add() otherwise); add() returns to the zone's adder
class PilotPointAdder : public detail::AdderBase<PilotPointAdder> {
public:
    PilotPointAdder(ObjectHandle adder, BackendProvider* backend, ControlZoneAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    // Ids of busbar sections or of buses of the bus-breaker view, in order
    PilotPointAdder& withBusbarSectionsOrBusesIds(const std::vector<std::string>& ids) {
        for (const std::string& id : ids) stringField(prop::EXT_SVC_PILOT_POINT_IDS, id);
        return *this;
    }
    PilotPointAdder& withTargetV(double targetV) { return doubleField(prop::EXT_SVC_TARGET_V, targetV); }

    ControlZoneAdder& add() {
        create();
        return parent_;
    }

private:
    ControlZoneAdder& parent_;
};

// A control unit of a new control zone: the id of its equipment (ValidationException from add()
// without one), participating unless told otherwise; add() returns to the zone's adder
class ControlUnitAdder : public detail::AdderBase<ControlUnitAdder> {
public:
    ControlUnitAdder(ObjectHandle adder, BackendProvider* backend, ControlZoneAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    ControlUnitAdder& withId(const std::string& id) { return stringField(prop::EXT_SVC_UNIT_ID, id); }
    ControlUnitAdder& withParticipate(bool participate) { return boolField(prop::EXT_SVC_UNIT_PARTICIPATE, participate); }

    ControlZoneAdder& add() {
        create();
        return parent_;
    }

private:
    ControlZoneAdder& parent_;
};

inline ControlZoneAdder SecondaryVoltageControlAdder::newControlZone() {
    return ControlZoneAdder(backend_->invoke(adder_, prop::CONTROL_ZONE), backend_, *this);
}

inline PilotPointAdder ControlZoneAdder::newPilotPoint() {
    return PilotPointAdder(backend_->invoke(adder_, prop::PILOT_POINT), backend_, *this);
}

inline ControlUnitAdder ControlZoneAdder::newControlUnit() {
    return ControlUnitAdder(backend_->invoke(adder_, prop::CONTROL_UNIT), backend_, *this);
}

} // namespace iidm
