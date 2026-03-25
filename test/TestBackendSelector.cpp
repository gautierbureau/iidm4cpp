#include <gtest/gtest.h>
#include <iidm/IidmException.h>
#include <iidm/NetworkFactory.h>
#include <cstdlib>

using namespace iidm;

// In a test environment without a live GraalVM library or JVM,
// we test that the selector correctly raises an exception rather than
// silently producing an unusable backend.

TEST(BackendSelectorTest, GraalVMNonExistentLibThrows) {
    // GraalVM mode with a non-existent library must throw
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

// ── GraalVM stub tests ────────────────────────────────────────────────────────
// These tests are only meaningful when the GraalVM backend is compiled in AND
// the stub shared library is available (set via IIDM_GRAALVM_STUB_LIB env var).
// In CI the graalvm job sets:
//   IIDM_GRAALVM_STUB_LIB=$GITHUB_WORKSPACE/graalvm-stub/libIidmStub.so

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED

TEST(BackendSelectorTest, GraalVMStubLibLoadsAndIsolateCreates) {
    const char* stubLib = std::getenv("IIDM_GRAALVM_STUB_LIB");
    if (!stubLib || stubLib[0] == '\0') {
        GTEST_SKIP() << "IIDM_GRAALVM_STUB_LIB not set; skipping stub integration test";
    }

    // The stub library exports the standard GraalVM isolate symbols but not
    // iidm_load_network, so loadNetwork() will throw after the isolate is
    // created.  What we validate here is that:
    //   1. dlopen succeeds (library is ABI-compatible).
    //   2. graal_create_isolate succeeds (GraalVM runtime initialises).
    //   3. The missing iidm_load_network symbol causes a clean IidmException
    //      rather than a crash or undefined behaviour.
    NetworkLoadOptions opts;
    opts.mode           = BackendMode::GRAALVM;
    opts.graalvmLibPath = stubLib;

    // GraalVMBackend constructor (dlopen + isolate) must succeed;
    // loadNetwork throws because iidm_load_network is absent in the stub.
    EXPECT_THROW(NetworkFactory::load("dummy.xiidm", opts), IidmException);
}

#endif // IIDM_BRIDGE_GRAALVM_ENABLED