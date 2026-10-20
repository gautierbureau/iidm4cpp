#include <iidm/InjectionObservability.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "ObservabilityQualityAccess.h"

namespace iidm {

InjectionObservability::InjectionObservability(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

bool InjectionObservability::isObservable() const {
    return backend_->getBool(handle_, prop::EXT_INJ_OBS_OBSERVABLE);
}

InjectionObservability& InjectionObservability::setObservable(bool observable) {
    backend_->setBool(handle_, prop::EXT_INJ_OBS_OBSERVABLE, observable);
    return *this;
}

iidm::optional<ObservabilityQuality> InjectionObservability::getQualityP() const {
    return detail::readQuality(backend_, handle_, prop::EXT_INJ_OBS_QUALITY_P);
}

iidm::optional<ObservabilityQuality> InjectionObservability::getQualityQ() const {
    return detail::readQuality(backend_, handle_, prop::EXT_INJ_OBS_QUALITY_Q);
}

iidm::optional<ObservabilityQuality> InjectionObservability::getQualityV() const {
    return detail::readQuality(backend_, handle_, prop::EXT_INJ_OBS_QUALITY_V);
}

InjectionObservability& InjectionObservability::setQualityP(double standardDeviation) {
    detail::writeQuality(backend_, handle_, prop::EXT_INJ_OBS_QUALITY_P, standardDeviation);
    return *this;
}

InjectionObservability& InjectionObservability::setQualityQ(double standardDeviation) {
    detail::writeQuality(backend_, handle_, prop::EXT_INJ_OBS_QUALITY_Q, standardDeviation);
    return *this;
}

InjectionObservability& InjectionObservability::setQualityV(double standardDeviation) {
    detail::writeQuality(backend_, handle_, prop::EXT_INJ_OBS_QUALITY_V, standardDeviation);
    return *this;
}

InjectionObservability& InjectionObservability::setQualityP(double standardDeviation, iidm::optional<bool> redundant) {
    detail::writeQuality(backend_, handle_, prop::EXT_INJ_OBS_QUALITY_P, standardDeviation, redundant);
    return *this;
}

InjectionObservability& InjectionObservability::setQualityQ(double standardDeviation, iidm::optional<bool> redundant) {
    detail::writeQuality(backend_, handle_, prop::EXT_INJ_OBS_QUALITY_Q, standardDeviation, redundant);
    return *this;
}

InjectionObservability& InjectionObservability::setQualityV(double standardDeviation, iidm::optional<bool> redundant) {
    detail::writeQuality(backend_, handle_, prop::EXT_INJ_OBS_QUALITY_V, standardDeviation, redundant);
    return *this;
}

} // namespace iidm
