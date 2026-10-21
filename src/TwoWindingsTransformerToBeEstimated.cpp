#include <iidm/TwoWindingsTransformerToBeEstimated.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

TwoWindingsTransformerToBeEstimated::TwoWindingsTransformerToBeEstimated(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
