#include <iidm/IdentifiableShortCircuit.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

IdentifiableShortCircuit::IdentifiableShortCircuit(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
