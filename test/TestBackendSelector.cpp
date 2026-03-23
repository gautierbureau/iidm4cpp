#include <gtest/gtest.h>
#include <iidm/IidmException.h>
#include <iidm/NetworkFactory.h>

using namespace iidm;

// In a test environment without a live GraalVM library or JVM,
// we test that the selector correctly raises an exception rather than
// silently producing an unusable backend.

TEST(BackendSelectorTest, AutoModeWithoutBackendsThrows) {
    // Set mode to GraalVM with a non-existent library — expect exception
    NetworkLoadOptions opts;
    opts.mode = BackendMode::GRAALVM;
    opts.graalvmLibPath = "/nonexistent/libpowsybl-iidm-native.so";

    EXPECT_THROW(NetworkFactory::load("dummy.xiidm", opts), IidmException);
}

TEST(BackendSelectorTest, JniModeWithoutJvmThrows) {
    // JNI mode without a running JVM must throw BackendNotAvailableException
    NetworkLoadOptions opts;
    opts.mode = BackendMode::JNI;

    // In a pure unit-test process there is no JVM
    EXPECT_THROW(NetworkFactory::load("dummy.xiidm", opts), IidmException);
}
