#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {

// A limit of the voltage angle difference between two terminals, from one to the other (powsybl
// VoltageAngleLimit, operational limits of the type VOLTAGE_ANGLE)
class VoltageAngleLimit : public detail::ObjectRef, public detail::PropertiesHolder<VoltageAngleLimit> {
public:
    static constexpr int TYPE = prop::VOLTAGE_ANGLE_LIMIT;

    VoltageAngleLimit() = default;
    explicit VoltageAngleLimit(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    std::string getId() const { return getString(prop::VAL_ID); }
    LimitType getLimitType() const { return static_cast<LimitType>(getInt(prop::LL_LIMIT_TYPE)); }
    Terminal getTerminalFrom() const;
    Terminal getTerminalTo() const;
    // Degrees; empty when unset
    iidm::optional<double> getLowLimit() const { return getOptDouble(prop::VAL_LOW_LIMIT); }
    iidm::optional<double> getHighLimit() const { return getOptDouble(prop::VAL_HIGH_LIMIT); }

    void remove();
};

// New voltage angle limit of a network (Network::newVoltageAngleLimit())
class VoltageAngleLimitAdder : public detail::AdderBase<VoltageAngleLimitAdder> {
public:
    VoltageAngleLimitAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    VoltageAngleLimitAdder& setId(const std::string& id) { return stringField(prop::VAL_ID, id); }
    VoltageAngleLimitAdder& from(const Terminal& terminal) { return relatedField(prop::REL_TERMINAL_FROM, detail::Access::handle(terminal)); }
    VoltageAngleLimitAdder& to(const Terminal& terminal) { return relatedField(prop::REL_TERMINAL_TO, detail::Access::handle(terminal)); }
    VoltageAngleLimitAdder& setLowLimit(double lowLimit) { return doubleField(prop::VAL_LOW_LIMIT, lowLimit); }
    VoltageAngleLimitAdder& setHighLimit(double highLimit) { return doubleField(prop::VAL_HIGH_LIMIT, highLimit); }
    VoltageAngleLimitAdder& setProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }

    VoltageAngleLimit add() { return VoltageAngleLimit(create(), backend_); }
};

} // namespace iidm
