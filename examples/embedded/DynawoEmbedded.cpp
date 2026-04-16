/**
 * Embedded mode example — C++ side.
 *
 * In embedded mode, the Java orchestrator has already loaded the network and
 * registered it via IidmBridgeRegistry.  This C++ entry point attaches to
 * the running JVM and reads the network directly from JVM memory.
 *
 * Loops generators (APC + CRC), HVDC lines (HADAPC + HOAR), static var
 * compensators (VPRC), and voltage levels (SlackTerminal) — all guarded with
 * has* checks so the same code runs cleanly on IEEE14 (no HVDC/SVC) and on
 * extensions_test.xiidm (no generators).
 */
#include <iidm/iidm.h>
#include <jni.h>
#include <iostream>

extern "C" {

void runEmbedded(const char* networkId) {
    try {
        iidm::NetworkLoadOptions opts;
        opts.mode         = iidm::BackendMode::JNI;
        opts.jniNetworkId = networkId ? networkId : "default";

        iidm::Network network = iidm::NetworkFactory::wrap(opts.jniNetworkId, opts);
        std::cout << "[C++] Network: " << network.getId() << "\n";

        // Generators: scale targetP and report extensions
        for (auto& gen : network.getGenerators()) {
            double targetP = gen.getTargetP();
            std::cout << "[C++] Generator " << gen.getId()
                      << " targetP=" << targetP << " MW\n";
            gen.setTargetP(targetP * 0.95);
            if (gen.hasActivePowerControl()) {
                double droop = gen.getActivePowerControl().getDroop();
                std::cout << "[C++]   activePowerControl.droop=" << droop << "\n";
                gen.getActivePowerControl().setDroop(2);
            }
            if (gen.hasCoordinatedReactiveControl()) {
                double qPct = gen.getCoordinatedReactiveControl().getQPercent();
                std::cout << "[C++]   coordinatedReactiveControl.qPercent=" << qPct << "\n";
                gen.getCoordinatedReactiveControl().setQPercent(qPct * 1.1);
            }
        }

        // HVDC lines: report extensions
        for (const auto& hvdc : network.getHvdcLines()) {
            std::cout << "[C++] HvdcLine " << hvdc.getId()
                      << " activePowerSetpoint=" << hvdc.getActivePowerSetpoint() << " MW\n";
            if (hvdc.hasHvdcAngleDroopActivePowerControl()) {
                auto ext = hvdc.getHvdcAngleDroopActivePowerControl();
                std::cout << "[C++]   hvdcAngleDroopActivePowerControl: droop=" << ext.getDroop()
                          << "  p0=" << ext.getP0()
                          << "  enabled=" << (ext.isEnabled() ? "true" : "false") << "\n";
            }
            if (hvdc.hasHvdcOperatorActivePowerRange()) {
                auto ext = hvdc.getHvdcOperatorActivePowerRange();
                std::cout << "[C++]   hvdcOperatorActivePowerRange: oprFromCS1toCS2="
                          << ext.getOprFromCS1toCS2()
                          << "  oprFromCS2toCS1=" << ext.getOprFromCS2toCS1() << "\n";
            }
        }

        // Static var compensators: report extensions
        for (const auto& svc : network.getStaticVarCompensators()) {
            std::cout << "[C++] StaticVarCompensator " << svc.getId() << "\n";
            if (svc.hasVoltagePerReactivePowerControl())
                std::cout << "[C++]   voltagePerReactivePowerControl.slope="
                          << svc.getVoltagePerReactivePowerControl().getSlope() << "\n";
        }

        // Voltage levels: report SlackTerminal
        for (const auto& vl : network.getVoltageLevels()) {
            if (vl.hasSlackTerminal())
                std::cout << "[C++] VoltageLevel " << vl.getId()
                          << " slackTerminal.busId="
                          << vl.getSlackTerminal().getTerminal().getBusId() << "\n";
        }

        std::cout << "[C++] Simulation complete.\n";

    } catch (const iidm::IidmException& ex) {
        std::cerr << "[C++] Error: " << ex.what() << "\n";
    }
}

JNIEXPORT void JNICALL Java_com_powsybl_iidmbridge_JavaLauncher_runEmbedded(JNIEnv* env, jclass, jstring jNetworkId) {
    const char* rawId = env->GetStringUTFChars(jNetworkId, nullptr);
    if (!rawId) return;
    std::string id(rawId);
    env->ReleaseStringUTFChars(jNetworkId, rawId);
    runEmbedded(id.c_str());
}

JNIEXPORT void JNICALL Java_JavaLauncher_runEmbedded(JNIEnv* env, jclass, jstring jNetworkId) {
    const char* rawId = env->GetStringUTFChars(jNetworkId, nullptr);
    if (!rawId) return;
    std::string id(rawId);
    env->ReleaseStringUTFChars(jNetworkId, rawId);
    runEmbedded(id.c_str());
}

} // extern "C"

int main() {
    std::cout << "This binary is intended to be called from Java via JNI.\n";
    std::cout << "To test standalone, use example_standalone instead.\n";
    return 0;
}
