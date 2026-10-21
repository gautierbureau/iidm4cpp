#include <iidm/Substation.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/TwoWindingsTransformer.h>
#include "Wrap.h"

namespace iidm {

iidm::optional<Country> Substation::getCountry() const {
    iidm::optional<std::string> name = backend_->getOptString(handle_, prop::COUNTRY);
    if (!name) return {};
    return countryFromName(*name);
}

Substation& Substation::setCountry(Country country) {
    backend_->setString(handle_, prop::COUNTRY, getCountryName(country));
    return *this;
}

iidm::optional<std::string> Substation::getTso() const {
    return backend_->getOptString(handle_, prop::SUBSTATION_TSO);
}

Substation& Substation::setTso(const std::string& tso) {
    backend_->setString(handle_, prop::SUBSTATION_TSO, tso);
    return *this;
}

std::vector<std::string> Substation::getGeographicalTags() const {
    return backend_->getStringList(handle_, prop::SUBSTATION_GEOGRAPHICAL_TAGS);
}

Substation& Substation::addGeographicalTag(const std::string& tag) {
    backend_->setString(handle_, prop::ADD_GEOGRAPHICAL_TAG, tag);
    return *this;
}

std::vector<VoltageLevel> Substation::getVoltageLevels() const {
    return detail::wrapAll<VoltageLevel>(backend_->getChildren(handle_, prop::VOLTAGE_LEVEL), backend_);
}

std::vector<TwoWindingsTransformer> Substation::getTwoWindingsTransformers() const {
    return detail::wrapAll<TwoWindingsTransformer>(backend_->getChildren(handle_, prop::TWO_WINDINGS_TRANSFORMER), backend_);
}

int Substation::getTwoWindingsTransformerCount() const {
    return detail::countOf(backend_, handle_, prop::TWO_WINDINGS_TRANSFORMER);
}

std::vector<ThreeWindingsTransformer> Substation::getThreeWindingsTransformers() const {
    return detail::wrapAll<ThreeWindingsTransformer>(backend_->getChildren(handle_, prop::THREE_WINDINGS_TRANSFORMER), backend_);
}

int Substation::getThreeWindingsTransformerCount() const {
    return detail::countOf(backend_, handle_, prop::THREE_WINDINGS_TRANSFORMER);
}

VoltageLevelAdder Substation::newVoltageLevel() {
    return VoltageLevelAdder(backend_->invoke(handle_, prop::VOLTAGE_LEVEL), backend_);
}

void Substation::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

} // namespace iidm
