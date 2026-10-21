#include <iidm/Overload.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

TemporaryLimit Overload::getTemporaryLimit() const {
    return TemporaryLimit(backend_->getRelated(handle_, prop::TEMPORARY_LIMIT), backend_);
}

} // namespace iidm
