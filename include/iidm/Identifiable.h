#pragma once

#include <iidm/DiscreteMeasurement.h>
#include <iidm/DiscreteMeasurementsAdder.h>
#include <iidm/DynamicModelInfo.h>
#include <iidm/DynamicModelInfoAdder.h>
#include <iidm/Enums.h>
#include <iidm/IdentifiableShortCircuit.h>
#include <iidm/IdentifiableShortCircuitAdder.h>
#include <iidm/IidmException.h>
#include <iidm/ObjectRef.h>
#include <iidm/OperatingStatus.h>
#include <iidm/OperatingStatusAdder.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;
class Network;

// A network object with an id (powsybl Identifiable): the base of every network object.
// Any of them converts to Identifiable; is<T>() and as<T>() go the other way.
class Identifiable : public detail::ObjectRef, public detail::PropertiesHolder<Identifiable> {
public:
    static constexpr int TYPE = prop::IDENTIFIABLE;

    Identifiable() = default;
    explicit Identifiable(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    std::string getId() const { return getString(prop::ID); }
    // Renames it (ValidationException when the id is taken)
    Identifiable& setId(const std::string& id) { return setString(*this, prop::ID, id); }
    // The name, or else the id
    std::string getNameOrId() const { return getString(prop::NAME); }
    // Empty when it has no name
    iidm::optional<std::string> getOptionalName() const { return getOptString(prop::NAME); }
    Identifiable& setName(const std::string& name) { return setString(*this, prop::NAME, name); }

    bool isFictitious() const { return getBool(prop::FICTITIOUS); }
    Identifiable& setFictitious(bool fictitious) { return setBool(*this, prop::FICTITIOUS, fictitious); }

    IdentifiableType getType() const { return static_cast<IdentifiableType>(getInt(prop::IDENTIFIABLE_TYPE)); }

    // Its network; the smallest network that holds it (a subnetwork, or else its network). Views
    // of the network: they do not own it (include <iidm/Network.h> to use them).
    Network getNetwork() const;
    Network getParentNetwork() const;

    // ── Aliases (other ids it can be found with) ─────────────────────────
    // Sorted
    std::vector<std::string> getAliases() const;
    bool hasAliases() const;
    // Empty when the alias has no type, or is not one of its aliases
    iidm::optional<std::string> getAliasType(const std::string& alias) const;
    iidm::optional<std::string> getAliasFromType(const std::string& aliasType) const;
    // aliasType "" for none. With ensureAliasUnicity, a free alias is derived from the given one
    // if it is already taken.
    Identifiable& addAlias(const std::string& alias, const std::string& aliasType = "",
                           bool ensureAliasUnicity = false);
    Identifiable& removeAlias(const std::string& alias);

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasDiscreteMeasurements() const;
    DiscreteMeasurements getDiscreteMeasurements() const;
    DiscreteMeasurementsAdder newDiscreteMeasurements();
    void removeDiscreteMeasurements();

    bool hasIdentifiableShortCircuit() const;
    IdentifiableShortCircuit getIdentifiableShortCircuit() const;
    IdentifiableShortCircuitAdder newIdentifiableShortCircuit();
    void removeIdentifiableShortCircuit();

    bool hasOperatingStatus() const;
    OperatingStatus getOperatingStatus() const;
    OperatingStatusAdder newOperatingStatus();
    void removeOperatingStatus();

    bool hasDynamicModelInfo() const;
    DynamicModelInfo getDynamicModelInfo() const;
    DynamicModelInfoAdder newDynamicModelInfo();
    void removeDynamicModelInfo();
};

} // namespace iidm
