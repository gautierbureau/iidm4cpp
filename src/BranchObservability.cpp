#include <iidm/BranchObservability.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "ObservabilityQualityAccess.h"

namespace iidm {

BranchObservability::BranchObservability(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle, backend), backend_(detail::orNullBackend(backend)) {}

bool BranchObservability::isObservable() const {
    return backend_->getBool(handle_, prop::EXT_BRANCH_OBS_OBSERVABLE);
}

BranchObservability& BranchObservability::setObservable(bool observable) {
    backend_->setBool(handle_, prop::EXT_BRANCH_OBS_OBSERVABLE, observable);
    return *this;
}

iidm::optional<ObservabilityQuality> BranchObservability::getQualityP1() const {
    return detail::readQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_P1);
}

iidm::optional<ObservabilityQuality> BranchObservability::getQualityP2() const {
    return detail::readQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_P2);
}

iidm::optional<ObservabilityQuality> BranchObservability::getQualityQ1() const {
    return detail::readQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_Q1);
}

iidm::optional<ObservabilityQuality> BranchObservability::getQualityQ2() const {
    return detail::readQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_Q2);
}

BranchObservability& BranchObservability::setQualityP1(double standardDeviation) {
    detail::writeQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_P1, standardDeviation);
    return *this;
}

BranchObservability& BranchObservability::setQualityP2(double standardDeviation) {
    detail::writeQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_P2, standardDeviation);
    return *this;
}

BranchObservability& BranchObservability::setQualityQ1(double standardDeviation) {
    detail::writeQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_Q1, standardDeviation);
    return *this;
}

BranchObservability& BranchObservability::setQualityQ2(double standardDeviation) {
    detail::writeQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_Q2, standardDeviation);
    return *this;
}

BranchObservability& BranchObservability::setQualityP1(double standardDeviation, iidm::optional<bool> redundant) {
    detail::writeQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_P1, standardDeviation, redundant);
    return *this;
}

BranchObservability& BranchObservability::setQualityP2(double standardDeviation, iidm::optional<bool> redundant) {
    detail::writeQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_P2, standardDeviation, redundant);
    return *this;
}

BranchObservability& BranchObservability::setQualityQ1(double standardDeviation, iidm::optional<bool> redundant) {
    detail::writeQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_Q1, standardDeviation, redundant);
    return *this;
}

BranchObservability& BranchObservability::setQualityQ2(double standardDeviation, iidm::optional<bool> redundant) {
    detail::writeQuality(backend_, handle_, prop::EXT_BRANCH_OBS_QUALITY_Q2, standardDeviation, redundant);
    return *this;
}

bool BranchObservability::operator==(const BranchObservability& other) const {
    return handle_ == other.handle_;
}

bool BranchObservability::operator!=(const BranchObservability& other) const {
    return !(*this == other);
}

} // namespace iidm
