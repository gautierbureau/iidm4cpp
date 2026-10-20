#include <iidm/Load.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double Load::getP0() const {
    return backend_->getDouble(handle_, prop::LOAD_P0);
}

Load& Load::setP0(double p0) {
    backend_->setDouble(handle_, prop::LOAD_P0, p0);
    return *this;
}

double Load::getQ0() const {
    return backend_->getDouble(handle_, prop::LOAD_Q0);
}

Load& Load::setQ0(double q0) {
    backend_->setDouble(handle_, prop::LOAD_Q0, q0);
    return *this;
}

LoadType Load::getLoadType() const {
    return static_cast<LoadType>(backend_->getInt(handle_, prop::LOAD_TYPE));
}

} // namespace iidm
