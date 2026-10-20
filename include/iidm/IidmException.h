#pragma once

#include <stdexcept>
#include <string>

namespace iidm {

// Base of every exception thrown by iidm4cpp. Both backends (GraalVM and JNI) report the same
// failure with the same exception type: Java-side errors never abort the process.
class IidmException : public std::runtime_error {
public:
    explicit IidmException(const std::string& message)
        : std::runtime_error(message) {}
};

class BackendNotAvailableException : public IidmException {
public:
    explicit BackendNotAvailableException(const std::string& message)
        : IidmException(message) {}
};

class InvalidHandleException : public IidmException {
public:
    explicit InvalidHandleException(const std::string& message)
        : IidmException(message) {}
};

class PropertyNotFoundException : public IidmException {
public:
    explicit PropertyNotFoundException(const std::string& message)
        : IidmException(message) {}
};

// The extension a property belongs to is not attached to the object
// (e.g. getActivePowerControl() on a generator without that extension).
class ExtensionNotFoundException : public PropertyNotFoundException {
public:
    explicit ExtensionNotFoundException(const std::string& message)
        : PropertyNotFoundException(message) {}
};

// The network model rejected an operation: invalid value (out of range, inconsistent
// limits...), object of the wrong voltage level, operation not supported by the topology...
class ValidationException : public IidmException {
public:
    explicit ValidationException(const std::string& message)
        : IidmException(message) {}
};

class NetworkNotFoundException : public IidmException {
public:
    explicit NetworkNotFoundException(const std::string& message)
        : IidmException(message) {}
};

} // namespace iidm
