#include <iidm/BusbarSection.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double BusbarSection::getV() const {
    return backend_->getDouble(handle_, prop::BBS_V);
}

double BusbarSection::getAngle() const {
    return backend_->getDouble(handle_, prop::BBS_ANGLE);
}

} // namespace iidm
