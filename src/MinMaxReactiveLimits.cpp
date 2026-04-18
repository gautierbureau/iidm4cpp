#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/BackendProvider.h>

namespace iidm {

MinMaxReactiveLimits::MinMaxReactiveLimits(ObjectHandle handle, BackendProvider* backend,
                                            int minQProp, int maxQProp)
    : handle_(handle), backend_(backend), minQProp_(minQProp), maxQProp_(maxQProp) {}

double MinMaxReactiveLimits::getMinQ() const {
    return backend_->getDouble(handle_, minQProp_);
}

double MinMaxReactiveLimits::getMaxQ() const {
    return backend_->getDouble(handle_, maxQProp_);
}

bool MinMaxReactiveLimits::operator==(const MinMaxReactiveLimits& other) const {
    return handle_ == other.handle_ && minQProp_ == other.minQProp_;
}

bool MinMaxReactiveLimits::operator!=(const MinMaxReactiveLimits& other) const {
    return !(*this == other);
}

} // namespace iidm
