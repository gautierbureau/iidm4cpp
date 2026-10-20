#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <string>

namespace iidm {
namespace detail {

// Base of the adders. An adder holds a handle on the Java (powsybl) adder; each of its setters
// sends one field with the code of the property it sets (GeneratorAdder::setTargetP sends
// GEN_TARGET_P), and create() adds the object (invoke(adder, ACT_ADD)). Nothing changes in the
// network before that. Validation is powsybl's: a missing or invalid field throws
// ValidationException from add(). Adders are move-only: two adders must not drive one Java adder.
template <class Self>
class AdderBase {
public:
    AdderBase(AdderBase&&) = default;
    AdderBase& operator=(AdderBase&&) = default;
    AdderBase(const AdderBase&) = delete;
    AdderBase& operator=(const AdderBase&) = delete;

protected:
    AdderBase(ObjectHandle adder, BackendProvider* backend)
        : adder_(adder, backend), backend_(orNullBackend(backend)) {}
    ~AdderBase() = default;

    // Distinct names: an overload set would send a string literal to the bool overload
    Self& doubleField(int code, double value)             { backend_->setDouble(adder_, code, value); return self(); }
    Self& intField(int code, int value)                   { backend_->setInt(adder_, code, value); return self(); }
    Self& boolField(int code, bool value)                 { backend_->setBool(adder_, code, value); return self(); }
    Self& stringField(int code, const std::string& value) { backend_->setString(adder_, code, value); return self(); }
    Self& relatedField(int code, ObjectHandle value)      { backend_->setRelated(adder_, code, value); return self(); }
    Self& keyedStringField(int code, const std::string& key, const std::string& value) {
        backend_->setKeyedString(adder_, code, key, value);
        return self();
    }

    // Adds the object: its handle (INVALID_HANDLE for a nested adder)
    ObjectHandle create() { return backend_->invoke(adder_, prop::ACT_ADD); }

    HandleRef adder_;
    BackendProvider* backend_;

private:
    Self& self() { return static_cast<Self&>(*this); }
};

template <class Self>
class IdentifiableAdder : public AdderBase<Self> {
public:
    Self& setId(const std::string& id)     { return this->stringField(prop::ID, id); }
    Self& setName(const std::string& name) { return this->stringField(prop::NAME, name); }
    // A free id is derived from the given one if it is already taken
    Self& setEnsureIdUnicity(bool ensureIdUnicity) { return this->boolField(prop::ENSURE_ID_UNICITY, ensureIdUnicity); }
    Self& setFictitious(bool fictitious)   { return this->boolField(prop::FICTITIOUS, fictitious); }

protected:
    IdentifiableAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase<Self>(adder, backend) {}
};

// Equipment with one terminal: where it is connected
template <class Self>
class InjectionAdder : public IdentifiableAdder<Self> {
public:
    // Bus-breaker topology: the bus it is connected to, and the bus it can be connected to
    Self& setBus(const std::string& busId)            { return this->stringField(prop::TERMINAL_BUS_BREAKER_BUS_ID, busId); }
    Self& setConnectableBus(const std::string& busId) { return this->stringField(prop::TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, busId); }
    // Node-breaker topology
    Self& setNode(int node)                           { return this->intField(prop::TERMINAL_NODE, node); }

protected:
    InjectionAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder<Self>(adder, backend) {}
};

} // namespace detail
} // namespace iidm
