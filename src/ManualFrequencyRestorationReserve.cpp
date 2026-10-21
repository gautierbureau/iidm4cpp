#include <iidm/ManualFrequencyRestorationReserve.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ManualFrequencyRestorationReserve::ManualFrequencyRestorationReserve(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
