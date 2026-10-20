#include <iidm/LccConverterStation.h>
#include <iidm/HvdcLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double LccConverterStation::getPowerFactor() const {
    return backend_->getDouble(handle_, prop::LCC_POWER_FACTOR);
}

} // namespace iidm
