#pragma once

#include <stdexcept>
#include <string>

namespace iidm {

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

class NetworkNotFoundException : public IidmException {
public:
    explicit NetworkNotFoundException(const std::string& message)
        : IidmException(message) {}
};

} // namespace iidm
