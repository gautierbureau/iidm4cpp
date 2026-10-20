#include <iidm/VscConverterStation.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/HvdcLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

bool VscConverterStation::hasMinMaxReactiveLimits() const {
    return backend_->getInt(handle_, prop::VSC_REACTIVE_LIMITS_KIND) == 1;
}

MinMaxReactiveLimits VscConverterStation::getMinMaxReactiveLimits() const {
    return MinMaxReactiveLimits(handle_, backend_, prop::VSC_MIN_Q, prop::VSC_MAX_Q);
}

bool VscConverterStation::hasReactiveCapabilityCurve() const {
    return backend_->getInt(handle_, prop::VSC_REACTIVE_LIMITS_KIND) == 2;
}

ReactiveCapabilityCurve VscConverterStation::getReactiveCapabilityCurve() const {
    return ReactiveCapabilityCurve(handle_, backend_);
}

} // namespace iidm
