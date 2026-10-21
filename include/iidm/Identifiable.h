#pragma once

#include <iidm/Enums.h>
#include <iidm/IidmException.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <utility>
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
    // An extension by its class, as in powsybl: getExtension<ActivePowerControl>(),
    // newExtension<ActivePowerControlAdder>().withDroop(4).add(). Include the extension's header to use
    // it. A host that powsybl does not allow for the extension never has it, and newExtension fails
    // there at run time (PropertyNotFoundException).
    template <class E> bool hasExtension() const { return static_cast<bool>(findExtension<E>()); }
    // Empty when absent
    template <class E> iidm::optional<E> findExtension() const {
        ObjectHandle extension = backend_->getRelated(handle_, E::TYPE);
        if (extension == INVALID_HANDLE) return {};
        return E(extension, backend_);
    }
    // ExtensionNotFoundException when absent
    template <class E> E getExtension() const {
        iidm::optional<E> extension = findExtension<E>();
        if (!extension) extensionNotFound(E::TYPE);
        return std::move(*extension);
    }
    // The extension it adds is the type that the adder's add() returns
    template <class A> A newExtension() {
        using E = decltype(std::declval<A&>().add());
        return A(backend_->invoke(handle_, E::TYPE), backend_);
    }
    template <class E> void removeExtension() { backend_->setRelated(handle_, E::TYPE, INVALID_HANDLE); }

private:
    [[noreturn]] void extensionNotFound(int type) const;
};

} // namespace iidm
