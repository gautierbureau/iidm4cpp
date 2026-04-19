#include <iidm/ActivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ActivePowerControl::ActivePowerControl(ObjectHandle handle, BackendProvider* backend,
                                       int droopCode, int participateCode)
    : handle_(handle), backend_(backend),
      droopCode_(droopCode), participateCode_(participateCode) {}

bool ActivePowerControl::isParticipate() const {
    return backend_->getBool(handle_, participateCode_);
}

ActivePowerControl& ActivePowerControl::setParticipate(bool participate) {
    backend_->setBool(handle_, participateCode_, participate);
    return *this;
}

double ActivePowerControl::getDroop() const {
    return backend_->getDouble(handle_, droopCode_);
}

ActivePowerControl& ActivePowerControl::setDroop(double droop) {
    backend_->setDouble(handle_, droopCode_, droop);
    return *this;
}

bool ActivePowerControl::operator==(const ActivePowerControl& other) const {
    return handle_ == other.handle_;
}

bool ActivePowerControl::operator!=(const ActivePowerControl& other) const {
    return !(*this == other);
}

} // namespace iidm
