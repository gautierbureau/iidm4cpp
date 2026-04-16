#include <iidm/HvdcOperatorActivePowerRange.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

HvdcOperatorActivePowerRange::HvdcOperatorActivePowerRange(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

double HvdcOperatorActivePowerRange::getOprFromCS1toCS2() const {
    return backend_->getDouble(handle_, prop::EXT_HOAR_OPR_CS1_TO_CS2);
}

HvdcOperatorActivePowerRange& HvdcOperatorActivePowerRange::setOprFromCS1toCS2(double value) {
    backend_->setDouble(handle_, prop::EXT_HOAR_OPR_CS1_TO_CS2, value);
    return *this;
}

double HvdcOperatorActivePowerRange::getOprFromCS2toCS1() const {
    return backend_->getDouble(handle_, prop::EXT_HOAR_OPR_CS2_TO_CS1);
}

HvdcOperatorActivePowerRange& HvdcOperatorActivePowerRange::setOprFromCS2toCS1(double value) {
    backend_->setDouble(handle_, prop::EXT_HOAR_OPR_CS2_TO_CS1, value);
    return *this;
}

bool HvdcOperatorActivePowerRange::operator==(const HvdcOperatorActivePowerRange& other) const {
    return handle_ == other.handle_;
}

bool HvdcOperatorActivePowerRange::operator!=(const HvdcOperatorActivePowerRange& other) const {
    return !(*this == other);
}

} // namespace iidm
