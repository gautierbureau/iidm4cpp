#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The LoadDetail extension of a load: its fixed and variable parts. An object of its own, from
// Load::getExtension<LoadDetail>().
class LoadDetail : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::LOAD_DETAIL;

    LoadDetail() = default;
    explicit LoadDetail(ObjectHandle handle, BackendProvider* backend);

    // MW
    double getFixedActivePower() const { return getDouble(prop::EXT_LD_FIXED_ACTIVE_POWER); }
    LoadDetail& setFixedActivePower(double fixedActivePower) {
        return setDouble(*this, prop::EXT_LD_FIXED_ACTIVE_POWER, fixedActivePower);
    }

    // MVar
    double getFixedReactivePower() const { return getDouble(prop::EXT_LD_FIXED_REACTIVE_POWER); }
    LoadDetail& setFixedReactivePower(double fixedReactivePower) {
        return setDouble(*this, prop::EXT_LD_FIXED_REACTIVE_POWER, fixedReactivePower);
    }

    // MW
    double getVariableActivePower() const { return getDouble(prop::EXT_LD_VARIABLE_ACTIVE_POWER); }
    LoadDetail& setVariableActivePower(double variableActivePower) {
        return setDouble(*this, prop::EXT_LD_VARIABLE_ACTIVE_POWER, variableActivePower);
    }

    // MVar
    double getVariableReactivePower() const { return getDouble(prop::EXT_LD_VARIABLE_REACTIVE_POWER); }
    LoadDetail& setVariableReactivePower(double variableReactivePower) {
        return setDouble(*this, prop::EXT_LD_VARIABLE_REACTIVE_POWER, variableReactivePower);
    }
};

} // namespace iidm
