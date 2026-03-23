#include <iidm/TwoWindingsTransformer.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

TwoWindingsTransformer::TwoWindingsTransformer(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string TwoWindingsTransformer::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string TwoWindingsTransformer::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double TwoWindingsTransformer::getR() const {
    return backend_->getDouble(handle_, prop::TWO_WT_R);
}

double TwoWindingsTransformer::getX() const {
    return backend_->getDouble(handle_, prop::TWO_WT_X);
}

double TwoWindingsTransformer::getG() const {
    return backend_->getDouble(handle_, prop::TWO_WT_G);
}

double TwoWindingsTransformer::getB() const {
    return backend_->getDouble(handle_, prop::TWO_WT_B);
}

double TwoWindingsTransformer::getRatedU1() const {
    return backend_->getDouble(handle_, prop::TWO_WT_RATED_U1);
}

double TwoWindingsTransformer::getRatedU2() const {
    return backend_->getDouble(handle_, prop::TWO_WT_RATED_U2);
}

std::optional<double> TwoWindingsTransformer::getRatedS() const {
    return backend_->getOptDouble(handle_, prop::TWO_WT_RATED_S);
}

Terminal TwoWindingsTransformer::getTerminal1() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL1);
    return Terminal(termHandle, backend_);
}

Terminal TwoWindingsTransformer::getTerminal2() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL2);
    return Terminal(termHandle, backend_);
}

bool TwoWindingsTransformer::operator==(const TwoWindingsTransformer& other) const {
    return handle_ == other.handle_;
}

bool TwoWindingsTransformer::operator!=(const TwoWindingsTransformer& other) const {
    return !(*this == other);
}

} // namespace iidm
