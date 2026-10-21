#include <iidm/GeneratorEntsoeCategory.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

GeneratorEntsoeCategory::GeneratorEntsoeCategory(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

int GeneratorEntsoeCategory::getCode() const {
    return backend_->getInt(handle_, prop::EXT_GEC_CODE);
}

GeneratorEntsoeCategory& GeneratorEntsoeCategory::setCode(int code) {
    backend_->setInt(handle_, prop::EXT_GEC_CODE, code);
    return *this;
}

} // namespace iidm
