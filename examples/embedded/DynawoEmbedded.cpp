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
#include "../ExtensionEdits.h"
#include "../StateEstimationReport.h"
#include <jni.h>
#include <iostream>
#include <string>

// Returns an empty string on success, the error message otherwise.
static std::string runEmbedded(const char* networkId) {
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
            if (svc.isWithMode(iidm::RegulationMode::VOLTAGE_PER_REACTIVE_POWER))
                std::cout << "[C++]   voltagePerReactivePowerControl.slope="
                          << svc.getVoltageRegulation()->getSlope() << "\n";
        }

        // Voltage levels: report SlackTerminal
        for (const auto& vl : network.getVoltageLevels()) {
            if (!vl.hasSlackTerminal()) continue;
            iidm::optional<iidm::Terminal> slack = vl.getSlackTerminal().getTerminal();
            std::cout << "[C++] VoltageLevel " << vl.getId()
                      << " slackTerminal.busId=" << (slack ? slack->getBusId() : "-") << "\n";
        }

        // ActivePowerControl (all fields) and SlackTerminal.setTerminal
        example::editActivePowerControlAndSlackTerminal(network, "[C++] ");

        // State estimation extensions (measurements, observability, toBeEstimated, areas)
        example::reportStateEstimation(network, "[C++] ");

        std::cout << "[C++] Simulation complete.\n";
        return {};

    } catch (const std::exception& ex) {
        std::cerr << "[C++] Error: " << ex.what() << "\n";
        return ex.what()[0] ? ex.what() : "unknown error";
    }
}

// Re-raise C++ failures as a Java RuntimeException so the launcher (and CI) fail loudly.
static void runEmbeddedFromJava(JNIEnv* env, jstring jNetworkId) {
    const char* rawId = env->GetStringUTFChars(jNetworkId, nullptr);
    if (!rawId) return;
    std::string id(rawId);
    env->ReleaseStringUTFChars(jNetworkId, rawId);
    std::string error = runEmbedded(id.c_str());
    if (!error.empty()) {
        jclass rte = env->FindClass("java/lang/RuntimeException");
        if (rte) env->ThrowNew(rte, ("[C++] " + error).c_str());
    }
}

extern "C" {

JNIEXPORT void JNICALL Java_com_powsybl_iidmbridge_JavaLauncher_runEmbedded(JNIEnv* env, jclass, jstring jNetworkId) {
    runEmbeddedFromJava(env, jNetworkId);
}

JNIEXPORT void JNICALL Java_JavaLauncher_runEmbedded(JNIEnv* env, jclass, jstring jNetworkId) {
    runEmbeddedFromJava(env, jNetworkId);
}

} // extern "C"

int main() {
    std::cout << "This binary is intended to be called from Java via JNI.\n";
    std::cout << "To test standalone, use example_standalone instead.\n";
    return 0;
}
