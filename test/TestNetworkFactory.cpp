#include <gtest/gtest.h>
#include <iidm/NetworkFactory.h>
#include <iidm/IidmException.h>
#include "MockBackend.h"
#include <cstdlib>
#include <memory>

using namespace iidm;
using namespace iidm::test;

// Portable wrappers for environment variable manipulation.
// setenv/unsetenv are POSIX-only; MSVC uses _putenv_s.
#ifdef _WIN32
static void testSetenv(const char* key, const char* val) { _putenv_s(key, val); }
// Setting to "" is equivalent to unset for our BackendSelector check,
// which only acts when the value is "graalvm" or "jni".
static void testUnsetenv(const char* key)                { _putenv_s(key, ""); }
#else
static void testSetenv(const char* key, const char* val) { setenv(key, val, 1); }
static void testUnsetenv(const char* key)                { unsetenv(key); }
#endif

// ── NetworkFactory::wrap ──────────────────────────────────────────────────────

TEST(NetworkFactoryTest, WrapWithoutJvmThrows) {
    // wrap() forces JNI mode; no JVM is running in a unit-test process
    EXPECT_THROW(NetworkFactory::wrap(""), BackendNotAvailableException);
}

TEST(NetworkFactoryTest, WrapWithNetworkIdWithoutJvmThrows) {
    EXPECT_THROW(NetworkFactory::wrap("my-network"), BackendNotAvailableException);
}

// ── NetworkFactory::load AUTO mode ───────────────────────────────────────────

TEST(NetworkFactoryTest, LoadAutoModeWithoutJvmOrGraalVMThrows) {
    // AUTO mode: probes for JVM (none), falls through to GraalVM (not compiled) → throws
    NetworkLoadOptions opts;
    opts.mode = BackendMode::AUTO;
    EXPECT_THROW(NetworkFactory::load("dummy.xiidm", opts), IidmException);
}

// ── BackendSelector: IIDM_BRIDGE_MODE env-var override ───────────────────────

TEST(NetworkFactoryTest, EnvOverrideGraalvmThrows) {
    // Force GRAALVM via env even when caller passes JNI mode
    testSetenv("IIDM_BRIDGE_MODE", "graalvm");
    NetworkLoadOptions opts;
    opts.mode = BackendMode::JNI; // would normally try JNI; env overrides to GRAALVM
    EXPECT_THROW(NetworkFactory::load("dummy.xiidm", opts), IidmException);
    testUnsetenv("IIDM_BRIDGE_MODE");
}

TEST(NetworkFactoryTest, EnvOverrideJniThrows) {
    // Force JNI via env even when caller passes AUTO mode
    testSetenv("IIDM_BRIDGE_MODE", "jni");
    NetworkLoadOptions opts;
    opts.mode = BackendMode::AUTO;
    EXPECT_THROW(NetworkFactory::load("dummy.xiidm", opts), IidmException);
    testUnsetenv("IIDM_BRIDGE_MODE");
}

TEST(NetworkFactoryTest, EnvOverrideUnrecognizedValueIgnored) {
    // An unrecognised value leaves the effective mode unchanged (AUTO → throws)
    testSetenv("IIDM_BRIDGE_MODE", "invalid_backend");
    NetworkLoadOptions opts;
    opts.mode = BackendMode::AUTO;
    EXPECT_THROW(NetworkFactory::load("dummy.xiidm", opts), IidmException);
    testUnsetenv("IIDM_BRIDGE_MODE");
}

// ── NetworkFactory::createFromBackend + move semantics ───────────────────────

TEST(NetworkFactoryTest, CreateFromBackendReturnsValidNetwork) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}]   = "net1";
    b->strings[{1, prop::NAME}] = "Network 1";
    Network net = NetworkFactory::createFromBackend(std::move(b));
    EXPECT_EQ(net.getId(),   "net1");
    EXPECT_EQ(net.getName(), "Network 1");
}

TEST(NetworkFactoryTest, NetworkMoveAssignment) {
    auto b = std::make_unique<MockBackend>();
    b->strings[{1, prop::ID}] = "net2";
    Network net1 = NetworkFactory::createFromBackend(std::move(b));
    Network net2(std::move(net1));
    EXPECT_EQ(net2.getId(), "net2");
}
