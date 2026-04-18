#include <iidm/CurrentLimits.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

CurrentLimits::CurrentLimits(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

double CurrentLimits::getPermanentLimit() const {
    return backend_->getDouble(handle_, prop::CL_PERMANENT_LIMIT);
}

std::vector<TemporaryLimit> CurrentLimits::getTemporaryLimits() const {
    auto handles = backend_->getChildren(handle_, prop::TEMPORARY_LIMIT);
    std::vector<TemporaryLimit> result;
    result.reserve(handles.size());
    for (auto tlHandle : handles) {
        TemporaryLimit tl;
        tl.name               = backend_->getString(tlHandle, prop::TL_NAME);
        tl.value              = backend_->getDouble(tlHandle, prop::TL_VALUE);
        tl.acceptableDuration = backend_->getInt(tlHandle, prop::TL_ACCEPTABLE_DURATION);
        tl.fictitious         = backend_->getBool(tlHandle, prop::TL_FICTITIOUS);
        result.push_back(std::move(tl));
    }
    return result;
}

} // namespace iidm
