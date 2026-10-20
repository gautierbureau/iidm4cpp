#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/BackendProvider.h>

namespace iidm {

MinMaxReactiveLimits::MinMaxReactiveLimits(ObjectHandle handle, BackendProvider* backend,
                                            int minQProp, int maxQProp)
    : ObjectRef(handle, backend), minQProp_(minQProp), maxQProp_(maxQProp) {}

double MinMaxReactiveLimits::getMinQ() const {
    return backend_->getDouble(handle_, minQProp_);
}

double MinMaxReactiveLimits::getMaxQ() const {
    return backend_->getDouble(handle_, maxQProp_);
}

} // namespace iidm
