#include <iidm/Line.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double Line::getR() const {
    return backend_->getDouble(handle_, prop::LINE_R);
}

double Line::getX() const {
    return backend_->getDouble(handle_, prop::LINE_X);
}

double Line::getG1() const {
    return backend_->getDouble(handle_, prop::LINE_G1);
}

double Line::getB1() const {
    return backend_->getDouble(handle_, prop::LINE_B1);
}

double Line::getG2() const {
    return backend_->getDouble(handle_, prop::LINE_G2);
}

double Line::getB2() const {
    return backend_->getDouble(handle_, prop::LINE_B2);
}

} // namespace iidm
