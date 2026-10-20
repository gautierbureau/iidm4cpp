#include <iidm/Substation.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

iidm::optional<Country> Substation::getCountry() const {
    iidm::optional<std::string> name = backend_->getOptString(handle_, prop::COUNTRY);
    if (!name) return {};
    return countryFromName(*name);
}

iidm::optional<std::string> Substation::getTso() const {
    return backend_->getOptString(handle_, prop::SUBSTATION_TSO);
}

Substation& Substation::setTso(const std::string& tso) {
    backend_->setString(handle_, prop::SUBSTATION_TSO, tso);
    return *this;
}

std::vector<VoltageLevel> Substation::getVoltageLevels() const {
    return detail::wrapAll<VoltageLevel>(backend_->getChildren(handle_, prop::VOLTAGE_LEVEL), backend_);
}

VoltageLevelAdder Substation::newVoltageLevel() {
    return VoltageLevelAdder(backend_->invoke(handle_, prop::VOLTAGE_LEVEL), backend_);
}

void Substation::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

} // namespace iidm
