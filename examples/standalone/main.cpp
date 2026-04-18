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
#include <iidm/RatioTapChanger.h>
#include <iostream>
#include <string>

static void printNetwork(iidm::Network& network) {
    std::cout << "Network: " << network.getId()
              << " (" << network.getName() << ")\n\n";

    // Generators
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
        if (gen.hasMinMaxReactiveLimits()) {
            auto rl = gen.getMinMaxReactiveLimits();
            std::cout << "    minMaxReactiveLimits: minQ=" << rl.getMinQ()
                      << " maxQ=" << rl.getMaxQ() << "\n";
        }
        if (gen.hasReactiveCapabilityCurve()) {
            auto pts = gen.getReactiveCapabilityCurve().getPoints();
            std::cout << "    reactiveCapabilityCurve: " << pts.size() << " point(s)\n";
            for (const auto& pt : pts)
                std::cout << "      p=" << pt.p << " minQ=" << pt.minQ << " maxQ=" << pt.maxQ << "\n";
        }
    }

    // Two-windings transformers
    auto twoWTs = network.getTwoWindingsTransformers();
    std::cout << "\n" << twoWTs.size() << " two-windings transformer(s):\n";
    for (const auto& twt : twoWTs) {
        std::cout << "  " << twt.getId()
                  << "  R=" << twt.getR() << "  X=" << twt.getX()
                  << "  ratedU1=" << twt.getRatedU1() << " kV"
                  << "  ratedU2=" << twt.getRatedU2() << " kV\n";
        if (twt.hasRatioTapChanger()) {
            auto rtc = twt.getRatioTapChanger();
            std::cout << "    ratioTapChanger: tap=" << rtc->getTapPosition()
                      << " [" << rtc->getLowTapPosition() << ".." << rtc->getHighTapPosition() << "]"
                      << "  regulating=" << (rtc->isRegulating() ? "yes" : "no")
                      << "  steps=" << rtc->getAllSteps().size() << "\n";
        }
        if (twt.hasPhaseTapChanger()) {
            auto ptc = twt.getPhaseTapChanger();
            std::cout << "    phaseTapChanger: tap=" << ptc->getTapPosition()
                      << " [" << ptc->getLowTapPosition() << ".." << ptc->getHighTapPosition() << "]"
                      << "  steps=" << ptc->getAllSteps().size() << "\n";
        }
    }

    // Three-windings transformers
    auto threeWTs = network.getThreeWindingsTransformers();
    std::cout << "\n" << threeWTs.size() << " three-windings transformer(s):\n";
    for (const auto& twt : threeWTs) {
        std::cout << "  " << twt.getId() << "\n";
        auto printLeg = [](const char* name, const iidm::ThreeWindingsTransformer::Leg& leg) {
            std::cout << "    " << name
                      << ": R=" << leg.getR() << " X=" << leg.getX()
                      << " ratedU=" << leg.getRatedU() << " kV\n";
            if (leg.hasRatioTapChanger()) {
                iidm::RatioTapChanger rtc = leg.getRatioTapChanger();
                std::cout << "      ratioTapChanger: tap=" << rtc.getTapPosition()
                          << " steps=" << rtc.getAllSteps().size() << "\n";
            }
        };
        printLeg("leg1", twt.getLeg1());
        printLeg("leg2", twt.getLeg2());
        printLeg("leg3", twt.getLeg3());
    }

    // HVDC lines
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

    // Static var compensators
    auto svcs = network.getStaticVarCompensators();
    std::cout << "\n" << svcs.size() << " static var compensator(s):\n";
    for (const auto& svc : svcs) {
        std::cout << "  " << svc.getId()
                  << "  voltageSetpoint=" << svc.getVoltageSetpoint() << " kV\n";
        if (svc.hasVoltagePerReactivePowerControl())
            std::cout << "    voltagePerReactivePowerControl.slope="
                      << svc.getVoltagePerReactivePowerControl().getSlope() << "\n";
    }

    // Shunt compensators
    auto shunts = network.getShuntCompensators();
    std::cout << "\n" << shunts.size() << " shunt compensator(s):\n";
    for (const auto& sc : shunts) {
        std::cout << "  " << sc.getId()
                  << "  sections=" << sc.getSectionCount()
                  << "/" << sc.getMaximumSectionCount();
        if (sc.hasLinearModel())
            std::cout << "  [linear: B/section=" << sc.getBPerSection() << "]";
        if (sc.hasNonLinearModel()) {
            auto sections = sc.getNonLinearModel().getAllSections();
            std::cout << "  [non-linear: " << sections.size() << " section(s)]";
        }
        std::cout << "\n";
    }

    // Batteries
    auto batteries = network.getBatteries();
    if (!batteries.empty()) {
        std::cout << "\n" << batteries.size() << " batter(y/ies):\n";
        for (const auto& bat : batteries) {
            std::cout << "  " << bat.getId()
                      << "  targetP=" << bat.getTargetP() << " MW"
                      << "  targetQ=" << bat.getTargetQ() << " Mvar"
                      << "  minP=" << bat.getMinP() << " maxP=" << bat.getMaxP() << "\n";
        }
    }

    // Voltage levels
    auto voltageLevels = network.getVoltageLevels();
    std::cout << "\n" << voltageLevels.size() << " voltage level(s):\n";
    for (const auto& vl : voltageLevels) {
        std::cout << "  " << vl.getId()
                  << "  nominalV=" << vl.getNominalV() << " kV";
        if (vl.hasSlackTerminal())
            std::cout << "  slackTerminal.busId="
                      << vl.getSlackTerminal().getTerminal().getBusId();
        std::cout << "\n";
        // Report buses (bus-breaker view)
        auto buses = vl.getBusBreakerView().getBuses();
        if (!buses.empty()) {
            std::cout << "    buses (bus-breaker view):";
            for (const auto& bus : buses)
                std::cout << " " << bus.getId();
            std::cout << "\n";
        }
    }

    // Loads
    auto loads = network.getLoads();
    std::cout << "\n" << loads.size() << " load(s):\n";
    for (const auto& load : loads) {
        std::cout << "  " << load.getId()
                  << "  P0=" << load.getP0() << " MW"
                  << "  Q0=" << load.getQ0() << " Mvar\n";
    }

    // Lines
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
