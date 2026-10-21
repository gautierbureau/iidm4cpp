#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <chrono>
#include <cstdlib>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// A network used from several threads, on the real backends (see RealBackends.h): the same
// scenarios on GraalVM and on JNI, with the network test/data/areas_test.xiidm (the load L3 on VL3,
// P0 = 150). Each thread makes its calls through its own isolate thread or JNI environment.

using namespace iidm;
using iidm::test::envPath;

namespace {

constexpr int THREADS = 4;

void loadNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/areas_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/areas_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
}

// Runs check(thread number) on THREADS threads at once
template <class F>
void onThreads(F check) {
    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back([&check, t] {
            try {
                check(t);
            } catch (const std::exception& e) {
                ADD_FAILURE() << "thread " << t << ": " << e.what();
            }
        });
    }
    for (std::thread& thread : threads) thread.join();
}

// Reads at once: each thread finds its objects, navigates and releases them
void checkReads(BackendProvider& b) {
    const std::size_t handles = b.liveHandleCount();
    onThreads([&b](int) {
        for (int i = 0; i < 200; ++i) {
            Load l3(b.findById(prop::LOAD, "L3"), &b);
            EXPECT_DOUBLE_EQ(l3.getP0(), 150.0);
            EXPECT_EQ(l3.getTerminal().getVoltageLevel().getId(), "VL3");
        }
    });
    EXPECT_EQ(b.liveHandleCount(), handles);
}

// Objects found on one thread, used and released on others
void checkObjectsMovedToThreads(BackendProvider& b) {
    const std::size_t handles = b.liveHandleCount();
    std::vector<Load> loads;
    for (int t = 0; t < THREADS; ++t) loads.emplace_back(b.findById(prop::LOAD, "L3"), &b);
    onThreads([&loads](int t) {
        Load load = std::move(loads[t]);
        EXPECT_EQ(load.getId(), "L3");
    });
    EXPECT_EQ(b.liveHandleCount(), handles);
}

// Writes at once, each thread on its variant (powsybl's multi-thread variant access): each Java
// call runs on the Java thread of its own C++ thread
void checkVariantPerThread(BackendProvider& b) {
    Load l3(b.findById(prop::LOAD, "L3"), &b);
    VariantManager manager = l3.getNetwork().getVariantManager();
    std::vector<std::string> variants;
    for (int t = 0; t < THREADS; ++t) variants.push_back("T" + std::to_string(t));
    manager.cloneVariant(VariantManager::INITIAL_VARIANT_ID, variants);
    manager.allowVariantMultiThreadAccess(true);
    onThreads([&](int t) {
        manager.setWorkingVariant(variants[t]);
        for (int i = 0; i < 100; ++i) {
            l3.setP0(1000 * t + i);
            EXPECT_DOUBLE_EQ(l3.getP0(), 1000 * t + i);
        }
    });
    // This thread kept its working variant
    EXPECT_EQ(manager.getWorkingVariantId(), VariantManager::INITIAL_VARIANT_ID);
    EXPECT_DOUBLE_EQ(l3.getP0(), 150.0);
    manager.allowVariantMultiThreadAccess(false);
    for (int t = 0; t < THREADS; ++t) {
        manager.setWorkingVariant(variants[t]);
        EXPECT_DOUBLE_EQ(l3.getP0(), 1000 * t + 99);
    }
    manager.setWorkingVariant(VariantManager::INITIAL_VARIANT_ID);
}

void checkNetwork(BackendProvider& b) {
    loadNetwork(b);
    checkReads(b);
    checkObjectsMovedToThreads(b);
    checkVariantPerThread(b);
}

// Opened on a thread that ends, then used and closed on this one
void checkOpenedOnAnotherThread(const std::function<std::unique_ptr<BackendProvider>()>& open) {
    std::unique_ptr<BackendProvider> backend;
    std::thread([&] {
        try {
            backend = open();
            loadNetwork(*backend);
        } catch (const std::exception& e) {
            ADD_FAILURE() << e.what();
        }
    }).join();
    ASSERT_TRUE(backend);
    Load l3(backend->findById(prop::LOAD, "L3"), backend.get());
    EXPECT_DOUBLE_EQ(l3.getP0(), 150.0);
}

// Closed while a thread that used it is still running: the close does not wait for it
void checkClosedWhileAThreadRuns(std::unique_ptr<BackendProvider> backend) {
    loadNetwork(*backend);
    std::promise<void> used;
    std::promise<void> closed;
    std::thread worker([&] {
        try {
            Load l3(backend->findById(prop::LOAD, "L3"), backend.get());
            EXPECT_DOUBLE_EQ(l3.getP0(), 150.0);
        } catch (const std::exception& e) {
            ADD_FAILURE() << e.what();
        }
        used.set_value();
        closed.get_future().wait();
    });
    used.get_future().wait();
    std::future<void> closing = std::async(std::launch::async, [&backend] { backend.reset(); });
    if (closing.wait_for(std::chrono::seconds(60)) != std::future_status::ready) {
        ADD_FAILURE() << "Closing the network waits for the threads that used it";
        std::_Exit(1); // the close never returns
    }
    closed.set_value();
    worker.join(); // ends after the close: nothing left to detach
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(ThreadsTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
    const int handles = backend.javaHandleCount();
    onThreads([&backend](int) { EXPECT_EQ(Load(backend.findById(prop::LOAD, "L3"), &backend).getId(), "L3"); });
    EXPECT_EQ(backend.javaHandleCount(), handles); // released on their threads
}

TEST(ThreadsTest, GraalVMOpenedOnAnotherThread) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    checkOpenedOnAnotherThread([lib] { return std::unique_ptr<BackendProvider>(new GraalVMBackend(lib)); });
}

TEST(ThreadsTest, GraalVMClosedWhileAThreadRuns) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    checkClosedWhileAThreadRuns(std::unique_ptr<BackendProvider>(new GraalVMBackend(lib)));
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
namespace {

// The live Java threads of the main thread group, where the JVM puts the threads attached to it
int javaThreadCount() {
    JavaVM* jvm = nullptr;
    jsize count = 0;
    JNI_GetCreatedJavaVMs(&jvm, 1, &count);
    JNIEnv* env = nullptr;
    jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8);
    jclass thread = env->FindClass("java/lang/Thread");
    const jint threads = env->CallStaticIntMethod(thread, env->GetStaticMethodID(thread, "activeCount", "()I"));
    EXPECT_FALSE(env->ExceptionCheck());
    env->DeleteLocalRef(thread);
    return threads;
}

} // namespace

TEST(ThreadsTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("threads");
    checkNetwork(backend);
    // The threads the backend attached to the JVM were detached when they ended
    const int threads = javaThreadCount();
    onThreads([&backend](int) { EXPECT_EQ(Load(backend.findById(prop::LOAD, "L3"), &backend).getId(), "L3"); });
    EXPECT_EQ(javaThreadCount(), threads);
}

TEST(ThreadsTest, JNIOpenedOnAnotherThread) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    checkOpenedOnAnotherThread([] { return std::unique_ptr<BackendProvider>(new JNIBackend("threads_opened")); });
}

TEST(ThreadsTest, JNIClosedWhileAThreadRuns) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    checkClosedWhileAThreadRuns(std::unique_ptr<BackendProvider>(new JNIBackend("threads_closed")));
}
#endif
