#include "BridgeErrors.h"
#include <iidm/IidmException.h>

namespace iidm { namespace detail {

void throwBridgeError(int kind, const std::string& message) {
    switch (kind) {
        case BRIDGE_INVALID_HANDLE:      throw InvalidHandleException(message);
        case BRIDGE_PROPERTY_NOT_FOUND:  throw PropertyNotFoundException(message);
        case BRIDGE_VALIDATION:          throw ValidationException(message);
        case BRIDGE_NETWORK_NOT_FOUND:   throw NetworkNotFoundException(message);
        default:                         throw IidmException(message);
    }
}

} } // namespace iidm::detail
