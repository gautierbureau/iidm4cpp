#include <iidm/DynamicModelInfo.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

DynamicModelInfo::DynamicModelInfo(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::string DynamicModelInfo::getModelName() const {
    return backend_->getString(handle_, prop::EXT_DMI_MODEL_NAME);
}

DynamicModelInfo& DynamicModelInfo::setModelName(const std::string& modelName) {
    backend_->setString(handle_, prop::EXT_DMI_MODEL_NAME, modelName);
    return *this;
}

} // namespace iidm
