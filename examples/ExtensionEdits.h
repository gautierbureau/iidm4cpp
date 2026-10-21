#pragma once

// Reads and edits ActivePowerControl and SlackTerminal. Shared by the standalone (GraalVM) and
// embedded (JNI) examples so both backends run the same code path.
//
// Edits, visible afterwards on the Java side (see JavaLauncher):
//   - ActivePowerControl: participationFactor = 0.5, maxTargetP = generator maxP
//   - SlackTerminal: moved to the terminal of the first static var compensator of the voltage level

#include <iidm/iidm.h>
#include <iostream>
#include <string>

namespace example {

// The connectable of the slack terminal of the current variant, "-" when it has none
inline std::string slackConnectableId(const iidm::SlackTerminal& slackTerminal) {
    iidm::optional<iidm::Terminal> terminal = slackTerminal.getTerminal();
    return terminal ? terminal->getConnectable().getId() : "-";
}

// The bus view bus that the slack terminal of the current variant can connect to: "" when it has
// none, "-" when there is no slack terminal
inline std::string slackBusId(const iidm::SlackTerminal& slackTerminal) {
    iidm::optional<iidm::Terminal> terminal = slackTerminal.getTerminal();
    if (!terminal) return "-";
    iidm::optional<iidm::Bus> bus = terminal->getBusView().getConnectableBus();
    return bus ? bus->getId() : "";
}

inline void editActivePowerControlAndSlackTerminal(iidm::Network& network, const std::string& prefix) {
    for (auto& gen : network.getGenerators()) {
        if (!gen.hasExtension<iidm::ActivePowerControl>()) continue;
        auto apc = gen.getExtension<iidm::ActivePowerControl>();
        auto minTargetP = apc.getMinTargetP();
        auto maxTargetP = apc.getMaxTargetP();
        std::cout << prefix << "Generator " << gen.getId() << " activePowerControl:"
                  << " participationFactor=" << apc.getParticipationFactor()
                  << " minTargetP=" << (minTargetP ? std::to_string(*minTargetP) : "-")
                  << " maxTargetP=" << (maxTargetP ? std::to_string(*maxTargetP) : "-") << "\n";
        apc.setParticipationFactor(0.5).setMaxTargetP(gen.getMaxP());
    }
    for (const auto& vl : network.getVoltageLevels()) {
        if (!vl.hasExtension<iidm::SlackTerminal>()) continue;
        auto st = vl.getExtension<iidm::SlackTerminal>();
        std::cout << prefix << "VoltageLevel " << vl.getId() << " slackTerminal=" << slackConnectableId(st)
                  << " empty=" << (st.isEmpty() ? "true" : "false") << "\n";
        for (const auto& svc : vl.getStaticVarCompensators()) {
            st.setTerminal(svc.getTerminal());
            std::cout << prefix << "  slackTerminal moved to " << slackConnectableId(st) << "\n";
            break;
        }
    }
}

} // namespace example
