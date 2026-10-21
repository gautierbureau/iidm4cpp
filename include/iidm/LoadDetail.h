#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The LoadDetail extension of a load: its fixed and variable parts. An object of its own, from
// Load::getLoadDetail().
class LoadDetail : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::LOAD_DETAIL;

    LoadDetail() = default;
    explicit LoadDetail(ObjectHandle handle, BackendProvider* backend);

    // MW
    double getFixedActivePower() const;
    LoadDetail& setFixedActivePower(double fixedActivePower);

    // MVar
    double getFixedReactivePower() const;
    LoadDetail& setFixedReactivePower(double fixedReactivePower);

    // MW
    double getVariableActivePower() const;
    LoadDetail& setVariableActivePower(double variableActivePower);

    // MVar
    double getVariableReactivePower() const;
    LoadDetail& setVariableReactivePower(double variableReactivePower);
};

} // namespace iidm
