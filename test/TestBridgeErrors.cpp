#include <gtest/gtest.h>
#include <iidm/IidmException.h>
#include "BridgeErrors.h"
#include "PropertyCodesCheck.h"
#include <string>

using namespace iidm;
using namespace iidm::detail;

// Each Java error kind maps to its C++ exception type, and keeps the message.

TEST(BridgeErrorsTest, InvalidHandle) {
    EXPECT_THROW(throwBridgeError(BRIDGE_INVALID_HANDLE, "h"), InvalidHandleException);
}

TEST(BridgeErrorsTest, PropertyNotFound) {
    EXPECT_THROW(throwBridgeError(BRIDGE_PROPERTY_NOT_FOUND, "p"), PropertyNotFoundException);
}

TEST(BridgeErrorsTest, Validation) {
    EXPECT_THROW(throwBridgeError(BRIDGE_VALIDATION, "v"), ValidationException);
}

TEST(BridgeErrorsTest, NetworkNotFound) {
    EXPECT_THROW(throwBridgeError(BRIDGE_NETWORK_NOT_FOUND, "n"), NetworkNotFoundException);
}

TEST(BridgeErrorsTest, GenericAndUnknownKinds) {
    EXPECT_THROW(throwBridgeError(BRIDGE_GENERIC, "g"), IidmException);
    EXPECT_THROW(throwBridgeError(42, "g"), IidmException);
}

TEST(BridgeErrorsTest, MessageIsKept) {
    try {
        throwBridgeError(BRIDGE_VALIDATION, "maxTargetP value (500.0) is not between minP and maxP");
        FAIL() << "no exception";
    } catch (const IidmException& e) {
        EXPECT_STREQ(e.what(), "maxTargetP value (500.0) is not between minP and maxP");
    }
}

// A backend opens only on a Java side built from the property codes of the library
TEST(BridgeErrorsTest, PropertyCodesCheck) {
    EXPECT_NO_THROW(checkPropertyCodes(propertyCodesHash(), "The jar"));
    try {
        checkPropertyCodes(propertyCodesHash() + 1, "The jar");
        ADD_FAILURE() << "Java code built from other codes accepted";
    } catch (const BackendNotAvailableException& e) {
        EXPECT_EQ(std::string(e.what()).find("The jar was built from other property codes"), 0u) << e.what();
    }
    // A Java side older than the check
    EXPECT_THROW(checkPropertyCodes(0, "The jar"), BackendNotAvailableException);
}
