#include <iidm/InternalConnection.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

InternalConnection::InternalConnection(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
