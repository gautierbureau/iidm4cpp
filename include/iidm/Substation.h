#pragma once

#include <iidm/Container.h>
#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
#include <iidm/OverloadManagementSystem.h>
#include <iidm/OverloadManagementSystemAdder.h>
#include <iidm/SubstationPosition.h>
#include <iidm/SubstationPositionAdder.h>
#include <iidm/VoltageLevel.h>
#include <iidm/VoltageLevelAdder.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class TwoWindingsTransformer;
class ThreeWindingsTransformer;

class Substation : public Identifiable, public detail::ContainerMethods<Substation> {
public:
    static constexpr int TYPE = prop::SUBSTATION;

    Substation() = default;
    explicit Substation(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    iidm::optional<Country> getCountry() const;
    Substation& setCountry(Country country);

    // Empty when undefined
    iidm::optional<std::string> getTso() const;
    Substation& setTso(const std::string& tso);

    // Sorted
    std::vector<std::string> getGeographicalTags() const;
    Substation& addGeographicalTag(const std::string& tag);

    std::vector<VoltageLevel> getVoltageLevels() const;
    std::vector<TwoWindingsTransformer> getTwoWindingsTransformers() const;
    int getTwoWindingsTransformerCount() const;
    std::vector<ThreeWindingsTransformer> getThreeWindingsTransformers() const;
    int getThreeWindingsTransformerCount() const;
    VoltageLevelAdder newVoltageLevel();
    std::vector<OverloadManagementSystem> getOverloadManagementSystems() const;
    int getOverloadManagementSystemCount() const;
    OverloadManagementSystemAdder newOverloadManagementSystem();

    // Removes the substation, its voltage levels and everything connected to them
    void remove();

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasSubstationPosition() const;
    SubstationPosition getSubstationPosition() const;
    SubstationPositionAdder newSubstationPosition();
    void removeSubstationPosition();
};

} // namespace iidm
