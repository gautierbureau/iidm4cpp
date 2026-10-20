#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {
namespace detail {

// powsybl properties (string values by name) of an object: identifiables, and the other
// powsybl PropertiesHolder objects.
template <class Self>
class PropertiesHolder {
public:
    // Whether it has any property
    bool hasProperty() const { return !getPropertyNames().empty(); }
    bool hasProperty(const std::string& key) const { return static_cast<bool>(getProperty(key)); }

    // Empty when there is no such property
    iidm::optional<std::string> getProperty(const std::string& key) const {
        return backend()->getKeyedString(handle(), prop::PROPERTY, key);
    }
    std::string getProperty(const std::string& key, const std::string& defaultValue) const {
        iidm::optional<std::string> value = getProperty(key);
        return value ? *value : defaultValue;
    }

    Self& setProperty(const std::string& key, const std::string& value) {
        backend()->setKeyedString(handle(), prop::PROPERTY, key, value);
        return static_cast<Self&>(*this);
    }

    // Whether there was such a property
    bool removeProperty(const std::string& key) {
        const bool present = hasProperty(key);
        backend()->setKeyedString(handle(), prop::PROPERTY, key, iidm::optional<std::string>());
        return present;
    }

    // Sorted
    std::vector<std::string> getPropertyNames() const {
        return backend()->getStringList(handle(), prop::PROPERTY_NAMES);
    }

protected:
    PropertiesHolder() = default;
    ~PropertiesHolder() = default;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
};

} // namespace detail
} // namespace iidm
