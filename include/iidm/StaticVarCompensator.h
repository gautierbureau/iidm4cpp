#pragma once

#include <iidm/Injection.h>
#include <iidm/VoltageRegulationHolder.h>

namespace iidm {

// Its voltage regulation (and the former VoltagePerReactivePowerControl slope) is a
// VoltageRegulation object: see VoltageRegulationHolderMethods
class StaticVarCompensator : public Injection, public detail::VoltageRegulationHolderMethods<StaticVarCompensator> {
public:
    static constexpr int TYPE = prop::STATIC_VAR_COMPENSATOR;

    StaticVarCompensator() = default;
    explicit StaticVarCompensator(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getBmin() const { return getDouble(prop::SVC_B_MIN); }
    StaticVarCompensator& setBmin(double bMin) { return setDouble(*this, prop::SVC_B_MIN, bMin); }

    double getBmax() const { return getDouble(prop::SVC_B_MAX); }
    StaticVarCompensator& setBmax(double bMax) { return setDouble(*this, prop::SVC_B_MAX, bMax); }
};

} // namespace iidm
