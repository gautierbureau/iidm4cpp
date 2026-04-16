/**
 * Standalone mode example.
 *
 * Loads an XIIDM network file using the GraalVM native image backend and
 * prints generators, HVDC lines, static var compensators, and voltage levels,
 * exercising all supported extensions via has* guards.
 *
 * Prerequisite:
 *   - libpowsybl-iidm-native.so on LD_LIBRARY_PATH, or
 *   - IIDM_BRIDGE_GRAALVM_LIB set to its full path.
 *
 * Usage:
 *   ./example_standalone path/to/network.xiidm
 */
#include <iidm/iidm.h>
#include <iostream>
#include <string>

static void printNetwork(iidm::Network& network) {
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
        if (gen.hasActivePowerControl())
            std::cout << "    activePowerControl.droop="
                      << gen.getActivePowerControl().getDroop() << "\n";
        if (gen.hasCoordinatedReactiveControl())
            std::cout << "    coordinatedReactiveControl.qPercent="
                      << gen.getCoordinatedReactiveControl().getQPercent() << "\n";
    }

    auto hvdcLines = network.getHvdcLines();
    std::cout << "\n" << hvdcLines.size() << " HVDC line(s):\n";
    for (const auto& hvdc : hvdcLines) {
        std::cout << "  " << hvdc.getId()
                  << "  activePowerSetpoint=" << hvdc.getActivePowerSetpoint() << " MW"
                  << "  maxP=" << hvdc.getMaxP() << " MW\n";
        if (hvdc.hasHvdcAngleDroopActivePowerControl()) {
            auto ext = hvdc.getHvdcAngleDroopActivePowerControl();
            std::cout << "    hvdcAngleDroopActivePowerControl: droop=" << ext.getDroop()
                      << "  p0=" << ext.getP0()
                      << "  enabled=" << (ext.isEnabled() ? "true" : "false") << "\n";
        }
        if (hvdc.hasHvdcOperatorActivePowerRange()) {
            auto ext = hvdc.getHvdcOperatorActivePowerRange();
            std::cout << "    hvdcOperatorActivePowerRange: oprFromCS1toCS2=" << ext.getOprFromCS1toCS2()
                      << "  oprFromCS2toCS1=" << ext.getOprFromCS2toCS1() << "\n";
        }
    }

    auto svcs = network.getStaticVarCompensators();
    std::cout << "\n" << svcs.size() << " static var compensator(s):\n";
    for (const auto& svc : svcs) {
        std::cout << "  " << svc.getId()
                  << "  voltageSetpoint=" << svc.getVoltageSetpoint() << " kV\n";
        if (svc.hasVoltagePerReactivePowerControl())
            std::cout << "    voltagePerReactivePowerControl.slope="
                      << svc.getVoltagePerReactivePowerControl().getSlope() << "\n";
    }

    auto voltageLevels = network.getVoltageLevels();
    std::cout << "\n" << voltageLevels.size() << " voltage level(s):\n";
    for (const auto& vl : voltageLevels) {
        std::cout << "  " << vl.getId()
                  << "  nominalV=" << vl.getNominalV() << " kV";
        if (vl.hasSlackTerminal())
            std::cout << "  slackTerminal.busId="
                      << vl.getSlackTerminal().getTerminal().getBusId();
        std::cout << "\n";
    }

    auto loads = network.getLoads();
    std::cout << "\n" << loads.size() << " load(s):\n";
    for (const auto& load : loads) {
        std::cout << "  " << load.getId()
                  << "  P0=" << load.getP0() << " MW"
                  << "  Q0=" << load.getQ0() << " Mvar\n";
    }

    auto lines = network.getLines();
    std::cout << "\n" << lines.size() << " line(s):\n";
    for (const auto& line : lines) {
        std::cout << "  " << line.getId()
                  << "  R=" << line.getR()
                  << "  X=" << line.getX() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path-to-network.xiidm>\n";
        return 1;
    }

    try {
        iidm::NetworkLoadOptions opts;
        opts.mode = iidm::BackendMode::GRAALVM;

        iidm::Network network = iidm::NetworkFactory::load(argv[1], opts);
        printNetwork(network);

    } catch (const iidm::IidmException& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 2;
    }

    return 0;
}
