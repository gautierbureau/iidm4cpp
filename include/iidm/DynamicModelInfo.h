#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <string>

namespace iidm {

class BackendProvider;

// The DynamicModelInfo extension of an identifiable: the dynamic model it is simulated with. An
// object of its own, from Identifiable::getDynamicModelInfo().
class DynamicModelInfo : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::DYNAMIC_MODEL_INFO;

    DynamicModelInfo() = default;
    explicit DynamicModelInfo(ObjectHandle handle, BackendProvider* backend);

    // The name of the dynamic model
    std::string getModelName() const;
    DynamicModelInfo& setModelName(const std::string& modelName);
};

} // namespace iidm
