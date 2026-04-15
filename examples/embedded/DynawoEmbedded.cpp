/**
 * Embedded mode example — C++ side.
 *
 * In embedded mode, the Java orchestrator has already loaded the network and
 * registered it via IidmBridgeRegistry.  This C++ entry point attaches to
 * the running JVM and reads the network directly from JVM memory.
 *
 * The Java side (JavaLauncher.java) typically calls this function via JNI
 * after registering the network.
 */
#include <iidm/iidm.h>
#include <jni.h>
#include <iostream>

extern "C" {

/**
 * C entry point called from the Java orchestrator via JNI.
 *
 * @param networkId  the ID under which the network was registered in
 *                   IidmBridgeRegistry (e.g. "case1").
 */
void runEmbedded(const char* networkId) {
    try {
        iidm::NetworkLoadOptions opts;
        opts.mode        = iidm::BackendMode::JNI;
        opts.jniNetworkId = networkId ? networkId : "default";

        iidm::Network network = iidm::NetworkFactory::wrap(opts.jniNetworkId, opts);

        std::cout << "[C++] Network: " << network.getId() << "\n";

        // Example: read and update generators
        for (auto& gen : network.getGenerators()) {
            double targetP = gen.getTargetP();
            std::cout << "[C++] Generator " << gen.getId()
                      << " targetP=" << targetP << " MW\n";

            // Example simulation update: scale active power by 0.95
            gen.setTargetP(targetP * 0.95);
        }

        if (network.getGenerator("_GEN____3_SM").has_value()) {
            std::cout << "[C++] Generator _GEN____3_SM is get\n";
            if (network.getGenerator("_GEN____3_SM")->hasActivePowerControl()) {
                std::cout << "[C++] Generator _GEN____3_SM has active power control\n";
                double droop = network.getGenerator("_GEN____3_SM")->getActivePowerControl().getDroop();
                std::cout << "[C++]  droop=" << droop << "\n";
                network.getGenerator("_GEN____3_SM")->getActivePowerControl().setDroop(2);
            }
        }

        std::cout << "[C++] Simulation complete.\n";

    } catch (const iidm::IidmException& ex) {
        std::cerr << "[C++] Error: " << ex.what() << "\n";
    }
}

/**
 * JNI entry point called from JavaLauncher.java via System.loadLibrary.
 * Bridges the Java String networkId to the C-string runEmbedded() above.
 */
JNIEXPORT void JNICALL Java_com_powsybl_iidmbridge_JavaLauncher_runEmbedded(JNIEnv* env, jclass, jstring jNetworkId) {
    const char* rawId = env->GetStringUTFChars(jNetworkId, nullptr);
    if (!rawId) return;
    std::string id(rawId);
    env->ReleaseStringUTFChars(jNetworkId, rawId);
    runEmbedded(id.c_str());
}

/**
 * JNI entry point called from JavaLauncher.java via System.loadLibrary.
 * Bridges the Java String networkId to the C-string runEmbedded() above.
 */
JNIEXPORT void JNICALL Java_JavaLauncher_runEmbedded(JNIEnv* env, jclass, jstring jNetworkId) {
    const char* rawId = env->GetStringUTFChars(jNetworkId, nullptr);
    if (!rawId) return;
    std::string id(rawId);
    env->ReleaseStringUTFChars(jNetworkId, rawId);
    runEmbedded(id.c_str());
}

} // extern "C"

// Stand-alone test entry point (not used when called from JNI)
int main() {
    std::cout << "This binary is intended to be called from Java via JNI.\n";
    std::cout << "To test standalone, use example_standalone instead.\n";
    return 0;
}
