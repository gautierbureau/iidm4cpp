#pragma once

#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
#include <iidm/VoltageLevel.h>
#include <iidm/VoltageLevelAdder.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class Substation : public Identifiable {
public:
    static constexpr int TYPE = prop::SUBSTATION;

    Substation() = default;
    explicit Substation(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    iidm::optional<Country> getCountry() const;

    // Empty when undefined
    iidm::optional<std::string> getTso() const;
    Substation& setTso(const std::string& tso);

    std::vector<VoltageLevel> getVoltageLevels() const;
    VoltageLevelAdder newVoltageLevel();

    // Removes the substation, its voltage levels and everything connected to them
    void remove();
};

} // namespace iidm
