/**
 * Standalone mode example.
 *
 * Loads an XIIDM network file using the GraalVM native image backend and
 * prints the list of generators with their active power target.
 *
 * Prerequisite:
 *   - libpowsybl-iidm-native.so on LD_LIBRARY_PATH, or
 *   - IIDM_BRIDGE_GRAALVM_LIB set to its full path.
 *
 * Usage:
 *   ./example_standalone path/to/network.xiidm
 */
#include <iidm/iidm.h>
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path-to-network.xiidm>\n";
        return 1;
    }

    try {
        iidm::NetworkLoadOptions opts;
        opts.mode = iidm::BackendMode::GRAALVM;

        iidm::Network network = iidm::NetworkFactory::load(argv[1], opts);

        std::cout << "Network: " << network.getId()
                  << " (" << network.getName() << ")\n\n";

        auto generators = network.getGenerators();
        std::cout << generators.size() << " generator(s):\n";
        for (const auto& gen : generators) {
            std::cout << "  " << gen.getId()
                      << "  targetP=" << gen.getTargetP() << " MW"
                      << "  minP=" << gen.getMinP() << " MW"
                      << "  maxP=" << gen.getMaxP() << " MW"
                      << "  voltageRegulator=" << (gen.isVoltageRegulatorOn() ? "ON" : "OFF")
                      << "\n";
            if (gen.hasActivePowerControl()) {
                double droop = gen.getActivePowerControl().getDroop();
                std::cout << "    activePowerControl.droop=" << droop << "\n";
            }
            if (gen.hasCoordinatedReactiveControl()) {
                double qPct = gen.getCoordinatedReactiveControl().getQPercent();
                std::cout << "    coordinatedReactiveControl.qPercent=" << qPct << "\n";
            }
        }

        if (network.getGenerator("_GEN____3_SM").has_value()) {
            auto gen3 = network.getGenerator("_GEN____3_SM").value();
            if (gen3.hasActivePowerControl()) {
                double droop = gen3.getActivePowerControl().getDroop();
                std::cout << "Generator _GEN____3_SM activePowerControl.droop=" << droop << "\n";
            }
            if (gen3.hasCoordinatedReactiveControl()) {
                double qPct = gen3.getCoordinatedReactiveControl().getQPercent();
                std::cout << "Generator _GEN____3_SM coordinatedReactiveControl.qPercent=" << qPct << "\n";
            }
        }

        auto loads = network.getLoads();
        std::cout << "\n" << loads.size() << " load(s):\n";
        for (const auto& load : loads) {
            std::cout << "  " << load.getId()
                      << "  P0=" << load.getP0() << " MW"
                      << "  Q0=" << load.getQ0() << " Mvar"
                      << "\n";
        }

        auto lines = network.getLines();
        std::cout << "\n" << lines.size() << " line(s):\n";
        for (const auto& line : lines) {
            std::cout << "  " << line.getId()
                      << "  R=" << line.getR()
                      << "  X=" << line.getX()
                      << "\n";
        }

    } catch (const iidm::IidmException& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 2;
    }

    return 0;
}
