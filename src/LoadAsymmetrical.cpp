#include <iidm/LoadAsymmetrical.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

LoadAsymmetrical::LoadAsymmetrical(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

LoadConnectionType LoadAsymmetrical::getConnectionType() const {
    return static_cast<LoadConnectionType>(backend_->getInt(handle_, prop::EXT_LA_CONNECTION_TYPE));
}

LoadAsymmetrical& LoadAsymmetrical::setConnectionType(LoadConnectionType connectionType) {
    backend_->setInt(handle_, prop::EXT_LA_CONNECTION_TYPE, static_cast<int>(connectionType));
    return *this;
}

double LoadAsymmetrical::getDeltaPa() const {
    return backend_->getDouble(handle_, prop::EXT_LA_DELTA_PA);
}

LoadAsymmetrical& LoadAsymmetrical::setDeltaPa(double deltaPa) {
    backend_->setDouble(handle_, prop::EXT_LA_DELTA_PA, deltaPa);
    return *this;
}

double LoadAsymmetrical::getDeltaQa() const {
    return backend_->getDouble(handle_, prop::EXT_LA_DELTA_QA);
}

LoadAsymmetrical& LoadAsymmetrical::setDeltaQa(double deltaQa) {
    backend_->setDouble(handle_, prop::EXT_LA_DELTA_QA, deltaQa);
    return *this;
}

double LoadAsymmetrical::getDeltaPb() const {
    return backend_->getDouble(handle_, prop::EXT_LA_DELTA_PB);
}

LoadAsymmetrical& LoadAsymmetrical::setDeltaPb(double deltaPb) {
    backend_->setDouble(handle_, prop::EXT_LA_DELTA_PB, deltaPb);
    return *this;
}

double LoadAsymmetrical::getDeltaQb() const {
    return backend_->getDouble(handle_, prop::EXT_LA_DELTA_QB);
}

LoadAsymmetrical& LoadAsymmetrical::setDeltaQb(double deltaQb) {
    backend_->setDouble(handle_, prop::EXT_LA_DELTA_QB, deltaQb);
    return *this;
}

double LoadAsymmetrical::getDeltaPc() const {
    return backend_->getDouble(handle_, prop::EXT_LA_DELTA_PC);
}

LoadAsymmetrical& LoadAsymmetrical::setDeltaPc(double deltaPc) {
    backend_->setDouble(handle_, prop::EXT_LA_DELTA_PC, deltaPc);
    return *this;
}

double LoadAsymmetrical::getDeltaQc() const {
    return backend_->getDouble(handle_, prop::EXT_LA_DELTA_QC);
}

LoadAsymmetrical& LoadAsymmetrical::setDeltaQc(double deltaQc) {
    backend_->setDouble(handle_, prop::EXT_LA_DELTA_QC, deltaQc);
    return *this;
}

} // namespace iidm
