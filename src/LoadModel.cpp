#include <iidm/LoadModel.h>
#include <iidm/BackendProvider.h>

namespace iidm {

LoadModelType LoadModel::getType() const {
    return static_cast<LoadModelType>(backend_->getInt(handle_, prop::LOAD_MODEL_TYPE));
}

// ── ZipLoadModel ────────────────────────────────────────────────────────────────

double ZipLoadModel::getC0p() const {
    return backend_->getDouble(handle_, prop::ZIP_C0P);
}

ZipLoadModel& ZipLoadModel::setC0p(double c0p) {
    backend_->setDouble(handle_, prop::ZIP_C0P, c0p);
    return *this;
}

double ZipLoadModel::getC1p() const {
    return backend_->getDouble(handle_, prop::ZIP_C1P);
}

ZipLoadModel& ZipLoadModel::setC1p(double c1p) {
    backend_->setDouble(handle_, prop::ZIP_C1P, c1p);
    return *this;
}

double ZipLoadModel::getC2p() const {
    return backend_->getDouble(handle_, prop::ZIP_C2P);
}

ZipLoadModel& ZipLoadModel::setC2p(double c2p) {
    backend_->setDouble(handle_, prop::ZIP_C2P, c2p);
    return *this;
}

double ZipLoadModel::getC0q() const {
    return backend_->getDouble(handle_, prop::ZIP_C0Q);
}

ZipLoadModel& ZipLoadModel::setC0q(double c0q) {
    backend_->setDouble(handle_, prop::ZIP_C0Q, c0q);
    return *this;
}

double ZipLoadModel::getC1q() const {
    return backend_->getDouble(handle_, prop::ZIP_C1Q);
}

ZipLoadModel& ZipLoadModel::setC1q(double c1q) {
    backend_->setDouble(handle_, prop::ZIP_C1Q, c1q);
    return *this;
}

double ZipLoadModel::getC2q() const {
    return backend_->getDouble(handle_, prop::ZIP_C2Q);
}

ZipLoadModel& ZipLoadModel::setC2q(double c2q) {
    backend_->setDouble(handle_, prop::ZIP_C2Q, c2q);
    return *this;
}

// ── ExponentialLoadModel ────────────────────────────────────────────────────────

double ExponentialLoadModel::getNp() const {
    return backend_->getDouble(handle_, prop::EXP_NP);
}

ExponentialLoadModel& ExponentialLoadModel::setNp(double np) {
    backend_->setDouble(handle_, prop::EXP_NP, np);
    return *this;
}

double ExponentialLoadModel::getNq() const {
    return backend_->getDouble(handle_, prop::EXP_NQ);
}

ExponentialLoadModel& ExponentialLoadModel::setNq(double nq) {
    backend_->setDouble(handle_, prop::EXP_NQ, nq);
    return *this;
}

} // namespace iidm
