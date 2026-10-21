#pragma once

#include <iidm/Container.h>
#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
#include <iidm/OverloadManagementSystem.h>
#include <iidm/OverloadManagementSystemAdder.h>
#include <iidm/ThreeWindingsTransformerAdder.h>
#include <iidm/TwoWindingsTransformerAdder.h>
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
    iidm::optional<std::string> getTso() const { return getOptString(prop::SUBSTATION_TSO); }
    Substation& setTso(const std::string& tso) { return setString(*this, prop::SUBSTATION_TSO, tso); }

    // Sorted
    std::vector<std::string> getGeographicalTags() const;
    Substation& addGeographicalTag(const std::string& tag) { return setString(*this, prop::ADD_GEOGRAPHICAL_TAG, tag); }

    std::vector<VoltageLevel> getVoltageLevels() const;
    std::vector<TwoWindingsTransformer> getTwoWindingsTransformers() const;
    int getTwoWindingsTransformerCount() const;
    std::vector<ThreeWindingsTransformer> getThreeWindingsTransformers() const;
    int getThreeWindingsTransformerCount() const;
    VoltageLevelAdder newVoltageLevel();
    // Transformers between its voltage levels
    TwoWindingsTransformerAdder newTwoWindingsTransformer();
    // A copy of a transformer: its values and voltage levels; on add(), its operational limits too
    TwoWindingsTransformerAdder newTwoWindingsTransformer(const TwoWindingsTransformer& copied);
    ThreeWindingsTransformerAdder newThreeWindingsTransformer();
    std::vector<OverloadManagementSystem> getOverloadManagementSystems() const;
    int getOverloadManagementSystemCount() const;
    OverloadManagementSystemAdder newOverloadManagementSystem();

    // Removes the substation, its voltage levels and everything connected to them
    void remove();
};

} // namespace iidm
