#pragma once

#include <iidm/DiscreteMeasurement.h>
#include <iidm/Enums.h>
#include <iidm/IidmException.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// A network object with an id (powsybl Identifiable): the base of every network object.
// Any of them converts to Identifiable; is<T>() and as<T>() go the other way.
class Identifiable : public detail::ObjectRef, public detail::PropertiesHolder<Identifiable> {
public:
    static constexpr int TYPE = prop::IDENTIFIABLE;

    Identifiable() = default;
    explicit Identifiable(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    std::string getId() const;
    // The name, or else the id
    std::string getNameOrId() const;
    // Empty when it has no name
    iidm::optional<std::string> getOptionalName() const;
    Identifiable& setName(const std::string& name);

    bool isFictitious() const;
    Identifiable& setFictitious(bool fictitious);

    IdentifiableType getType() const;

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

    // ── Type ─────────────────────────────────────────────────────────────
    // Whether it is a T (Java instanceof): is<Generator>(), is<Injection>(), is<Branch>()...
    template <class T> bool is() const { return backend_->getBool(handle_, T::TYPE); }
    // This object as a T; PropertyNotFoundException when it is not one
    template <class T> T as() const;

    // ── Extensions ───────────────────────────────────────────────────────
    bool hasDiscreteMeasurements() const;
    DiscreteMeasurements getDiscreteMeasurements() const;
};

template <class T>
T Identifiable::as() const {
    if (!is<T>()) {
        throw PropertyNotFoundException(getId() + " is not of the requested type (code " + std::to_string(T::TYPE) + ")");
    }
    return T(handle_, backend_);
}

} // namespace iidm
