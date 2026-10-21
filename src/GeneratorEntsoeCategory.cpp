#include <iidm/GeneratorEntsoeCategory.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

GeneratorEntsoeCategory::GeneratorEntsoeCategory(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
