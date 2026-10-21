#include <iidm/LineFortescue.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

LineFortescue::LineFortescue(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double LineFortescue::getRz() const {
    return backend_->getDouble(handle_, prop::EXT_LINE_F_RZ);
}

LineFortescue& LineFortescue::setRz(double rz) {
    backend_->setDouble(handle_, prop::EXT_LINE_F_RZ, rz);
    return *this;
}

double LineFortescue::getXz() const {
    return backend_->getDouble(handle_, prop::EXT_LINE_F_XZ);
}

LineFortescue& LineFortescue::setXz(double xz) {
    backend_->setDouble(handle_, prop::EXT_LINE_F_XZ, xz);
    return *this;
}

double LineFortescue::getG1z() const {
    return backend_->getDouble(handle_, prop::EXT_LINE_F_G1Z);
}

LineFortescue& LineFortescue::setG1z(double g1z) {
    backend_->setDouble(handle_, prop::EXT_LINE_F_G1Z, g1z);
    return *this;
}

double LineFortescue::getB1z() const {
    return backend_->getDouble(handle_, prop::EXT_LINE_F_B1Z);
}

LineFortescue& LineFortescue::setB1z(double b1z) {
    backend_->setDouble(handle_, prop::EXT_LINE_F_B1Z, b1z);
    return *this;
}

double LineFortescue::getG2z() const {
    return backend_->getDouble(handle_, prop::EXT_LINE_F_G2Z);
}

LineFortescue& LineFortescue::setG2z(double g2z) {
    backend_->setDouble(handle_, prop::EXT_LINE_F_G2Z, g2z);
    return *this;
}

double LineFortescue::getB2z() const {
    return backend_->getDouble(handle_, prop::EXT_LINE_F_B2Z);
}

LineFortescue& LineFortescue::setB2z(double b2z) {
    backend_->setDouble(handle_, prop::EXT_LINE_F_B2Z, b2z);
    return *this;
}

bool LineFortescue::isOpenPhaseA() const {
    return backend_->getBool(handle_, prop::EXT_LINE_F_OPEN_PHASE_A);
}

LineFortescue& LineFortescue::setOpenPhaseA(bool openPhaseA) {
    backend_->setBool(handle_, prop::EXT_LINE_F_OPEN_PHASE_A, openPhaseA);
    return *this;
}

bool LineFortescue::isOpenPhaseB() const {
    return backend_->getBool(handle_, prop::EXT_LINE_F_OPEN_PHASE_B);
}

LineFortescue& LineFortescue::setOpenPhaseB(bool openPhaseB) {
    backend_->setBool(handle_, prop::EXT_LINE_F_OPEN_PHASE_B, openPhaseB);
    return *this;
}

bool LineFortescue::isOpenPhaseC() const {
    return backend_->getBool(handle_, prop::EXT_LINE_F_OPEN_PHASE_C);
}

LineFortescue& LineFortescue::setOpenPhaseC(bool openPhaseC) {
    backend_->setBool(handle_, prop::EXT_LINE_F_OPEN_PHASE_C, openPhaseC);
    return *this;
}

} // namespace iidm
