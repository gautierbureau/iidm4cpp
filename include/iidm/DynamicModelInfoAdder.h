#pragma once

#include <iidm/Adder.h>
#include <iidm/DynamicModelInfo.h>
#include <string>

namespace iidm {

// New DynamicModelInfo extension of an identifiable
class DynamicModelInfoAdder : public detail::AdderBase<DynamicModelInfoAdder> {
public:
    DynamicModelInfoAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    DynamicModelInfoAdder& setModelName(const std::string& modelName) { return stringField(prop::EXT_DMI_MODEL_NAME, modelName); }

    DynamicModelInfo add() { return DynamicModelInfo(create(), backend_); }
};

} // namespace iidm
