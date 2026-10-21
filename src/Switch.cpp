#include <iidm/Switch.h>
#include <iidm/VoltageLevel.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

VoltageLevel Switch::getVoltageLevel() const {
    return VoltageLevel(backend_->getRelated(handle_, prop::REL_VOLTAGE_LEVEL), backend_);
}

} // namespace iidm
