#include <iidm/LineFortescue.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

LineFortescue::LineFortescue(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
