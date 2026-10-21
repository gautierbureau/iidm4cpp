#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

CoordinatedReactiveControl::CoordinatedReactiveControl(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
