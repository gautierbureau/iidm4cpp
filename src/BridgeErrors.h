#pragma once

#include <string>

namespace iidm { namespace detail {

// Error kinds reported by the Java side (com.powsybl.iidmbridge.BridgeErrors, same values).
enum BridgeErrorKind {
    BRIDGE_OK                  = 0,
    BRIDGE_GENERIC             = 1, // IidmException
    BRIDGE_INVALID_HANDLE      = 2, // InvalidHandleException
    BRIDGE_PROPERTY_NOT_FOUND  = 3, // PropertyNotFoundException
    BRIDGE_VALIDATION          = 4, // ValidationException
    BRIDGE_NETWORK_NOT_FOUND   = 5  // NetworkNotFoundException
};

// Throws the C++ exception matching a Java error kind.
[[noreturn]] void throwBridgeError(int kind, const std::string& message);

} } // namespace iidm::detail
