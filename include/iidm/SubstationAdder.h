#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/Substation.h>
#include <string>

namespace iidm {

// New substation of a network (Network::newSubstation())
class SubstationAdder : public detail::IdentifiableAdder<SubstationAdder> {
public:
    SubstationAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder(adder, backend) {}

    SubstationAdder& setCountry(Country country)      { return stringField(prop::COUNTRY, getCountryName(country)); }
    SubstationAdder& setTso(const std::string& tso)   { return stringField(prop::SUBSTATION_TSO, tso); }

    Substation add() { return Substation(create(), backend_); }
};

} // namespace iidm
