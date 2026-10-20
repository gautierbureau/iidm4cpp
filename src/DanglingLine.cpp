#include <iidm/DanglingLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double DanglingLine::getP0() const {
    return backend_->getDouble(handle_, prop::DL_P0);
}

DanglingLine& DanglingLine::setP0(double p0) {
    backend_->setDouble(handle_, prop::DL_P0, p0);
    return *this;
}

double DanglingLine::getQ0() const {
    return backend_->getDouble(handle_, prop::DL_Q0);
}

DanglingLine& DanglingLine::setQ0(double q0) {
    backend_->setDouble(handle_, prop::DL_Q0, q0);
    return *this;
}

double DanglingLine::getR() const {
    return backend_->getDouble(handle_, prop::DL_R);
}

double DanglingLine::getX() const {
    return backend_->getDouble(handle_, prop::DL_X);
}

double DanglingLine::getG() const {
    return backend_->getDouble(handle_, prop::DL_G);
}

double DanglingLine::getB() const {
    return backend_->getDouble(handle_, prop::DL_B);
}

} // namespace iidm
