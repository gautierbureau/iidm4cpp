#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

// How the susceptance and conductance of a shunt compensator depend on its section count, in
// one of two types: ShuntCompensatorLinearModel (the same for each section) or
// ShuntCompensatorNonLinearModel (given section by section). ShuntCompensator::getModelType()
// or is<T>() tell which, as<T>() gives the model as that type.
class ShuntCompensatorModel : public detail::ObjectRef, public detail::PropertiesHolder<ShuntCompensatorModel> {
public:
    static constexpr int TYPE = prop::SHUNT_COMPENSATOR_MODEL;

    ShuntCompensatorModel() = default;
    explicit ShuntCompensatorModel(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}
};

} // namespace iidm
