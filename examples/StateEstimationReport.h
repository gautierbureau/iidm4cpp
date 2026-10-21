#pragma once

// Prints (and lightly edits) the state estimation extensions of a network. Shared by the
// standalone (GraalVM) and embedded (JNI) examples so both backends run the same code path.
//
// Edits, visible afterwards on the Java side (see JavaLauncher):
//   - G1_P measurement value set to 100.0 and marked valid
//   - T2 phase tap changer marked to be estimated
//   - G1 observability: standard deviation of V set to 0.9

#include <iidm/iidm.h>
#include <iostream>
#include <string>

namespace example {

inline const char* toString(iidm::MeasurementType type) {
    static const char* names[] = {"ANGLE", "ACTIVE_POWER", "APPARENT_POWER", "REACTIVE_POWER",
                                  "CURRENT", "VOLTAGE", "FREQUENCY", "OTHER"};
    return names[static_cast<int>(type)];
}

inline const char* toString(iidm::ObservabilityStatus status) {
    static const char* names[] = {"OBSERVABLE", "NON_OBSERVABLE", "BORDER"};
    return names[static_cast<int>(status)];
}

inline std::string toString(const iidm::optional<iidm::ObservabilityQuality>& quality) {
    if (!quality) return "-";
    std::string s = "sd=" + std::to_string(quality->getStandardDeviation());
    iidm::optional<bool> redundant = quality->isRedundant();
    if (redundant) s += *redundant ? " redundant" : " not-redundant";
    return s;
}

// A measurement may have no id
inline std::string toString(const iidm::optional<std::string>& id) {
    return id ? *id : "-";
}

inline void printMeasurements(const iidm::Identifiable& host, const std::string& prefix) {
    if (!host.hasExtension<iidm::Measurements>()) return;
    for (const auto& m : host.getExtension<iidm::Measurements>().getMeasurements()) {
        std::cout << prefix << "  measurement " << toString(m.getId()) << " " << toString(m.getType())
                  << " value=" << m.getValue() << " sd=" << m.getStandardDeviation()
                  << " valid=" << (m.isValueValid() ? "true" : "false");
        if (m.getSide()) std::cout << " side=" << (static_cast<int>(*m.getSide()) + 1);
        std::cout << "\n";
    }
}

inline void printDiscreteMeasurements(const iidm::Identifiable& host, const std::string& prefix) {
    if (!host.hasExtension<iidm::DiscreteMeasurements>()) return;
    for (const auto& m : host.getExtension<iidm::DiscreteMeasurements>().getDiscreteMeasurements()) {
        std::cout << prefix << "  discreteMeasurement " << toString(m.getId()) << " value=";
        switch (m.getValueType()) {
            case iidm::DiscreteMeasurementValueType::BOOLEAN: std::cout << (m.getValueAsBoolean() ? "true" : "false"); break;
            case iidm::DiscreteMeasurementValueType::INT:     std::cout << m.getValueAsInt(); break;
            case iidm::DiscreteMeasurementValueType::STRING:  std::cout << m.getValueAsString(); break;
        }
        std::cout << " valid=" << (m.isValueValid() ? "true" : "false") << "\n";
    }
}

inline void printInjectionObservability(const iidm::Identifiable& host, const std::string& prefix) {
    if (!host.hasExtension<iidm::InjectionObservability>()) return;
    auto obs = host.getExtension<iidm::InjectionObservability>();
    std::cout << prefix << "  injectionObservability observable=" << (obs.isObservable() ? "true" : "false")
              << " P[" << toString(obs.getQualityP()) << "]"
              << " Q[" << toString(obs.getQualityQ()) << "]"
              << " V[" << toString(obs.getQualityV()) << "]\n";
}

inline void reportStateEstimation(iidm::Network& network, const std::string& prefix) {
    for (auto& gen : network.getGenerators()) {
        std::cout << prefix << "Generator " << gen.getId() << "\n";
        printInjectionObservability(gen, prefix);
        printMeasurements(gen, prefix);
        if (gen.hasExtension<iidm::Measurements>()) {
            auto p = gen.getExtension<iidm::Measurements>().getMeasurement("G1_P");
            if (p) p->setValueAndValidity(100.0, true);
        }
        if (gen.hasExtension<iidm::InjectionObservability>())
            gen.getExtension<iidm::InjectionObservability>().setQualityV(0.9);
    }
    for (const auto& vl : network.getVoltageLevels()) {
        if (vl.getTopologyKind() == iidm::TopologyKind::NODE_BREAKER) {
            for (const auto& bbs : vl.getBusbarSections()) {
                std::cout << prefix << "BusbarSection " << bbs.getId() << "\n";
                printInjectionObservability(bbs, prefix);
                printMeasurements(bbs, prefix);
            }
        }
        if (vl.hasExtension<iidm::ObservabilityArea>()) {
            auto ext = vl.getExtension<iidm::ObservabilityArea>();
            std::cout << prefix << "VoltageLevel " << vl.getId() << " observabilityArea consistent="
                      << (ext.isConsistentWithTopology() ? "true" : "false") << "\n";
            for (const auto& area : ext.getObservabilityAreas()) {
                std::cout << prefix << "  area " << area.getAreaNumber() << " " << toString(area.getStatus())
                          << " terminals=" << area.getTerminals().size();
                if (!area.getNodes().empty()) {
                    std::cout << " nodes=";
                    for (int n : area.getNodes()) std::cout << n << " ";
                }
                for (const auto& bus : area.getBuses()) std::cout << " bus=" << bus.getId();
                std::cout << "\n";
            }
        }
    }
    for (const auto& line : network.getLines()) {
        std::cout << prefix << "Line " << line.getId() << "\n";
        if (line.hasExtension<iidm::BranchObservability>()) {
            auto obs = line.getExtension<iidm::BranchObservability>();
            std::cout << prefix << "  branchObservability observable=" << (obs.isObservable() ? "true" : "false")
                      << " P1[" << toString(obs.getQualityP1()) << "] P2[" << toString(obs.getQualityP2()) << "]"
                      << " Q1[" << toString(obs.getQualityQ1()) << "] Q2[" << toString(obs.getQualityQ2()) << "]\n";
        }
        printMeasurements(line, prefix);
    }
    for (auto& twt : network.getTwoWindingsTransformers()) {
        std::cout << prefix << "TwoWindingsTransformer " << twt.getId() << "\n";
        if (twt.hasExtension<iidm::TwoWindingsTransformerToBeEstimated>()) {
            auto tbe = twt.getExtension<iidm::TwoWindingsTransformerToBeEstimated>();
            std::cout << prefix << "  toBeEstimated rtc=" << tbe.shouldEstimateRatioTapChanger()
                      << " ptc=" << tbe.shouldEstimatePhaseTapChanger() << "\n";
            tbe.shouldEstimatePhaseTapChanger(true);
        }
        printDiscreteMeasurements(twt, prefix);
    }
    for (const auto& twt : network.getThreeWindingsTransformers()) {
        std::cout << prefix << "ThreeWindingsTransformer " << twt.getId() << "\n";
        if (twt.hasExtension<iidm::ThreeWindingsTransformerToBeEstimated>()) {
            auto tbe = twt.getExtension<iidm::ThreeWindingsTransformerToBeEstimated>();
            std::cout << prefix << "  toBeEstimated rtc=" << tbe.shouldEstimateRatioTapChanger1()
                      << tbe.shouldEstimateRatioTapChanger2() << tbe.shouldEstimateRatioTapChanger3()
                      << " ptc=" << tbe.shouldEstimatePhaseTapChanger1() << tbe.shouldEstimatePhaseTapChanger2()
                      << tbe.shouldEstimatePhaseTapChanger3() << "\n";
        }
    }
    auto sw = network.getSwitch("SW1");
    if (sw) {
        std::cout << prefix << "Switch " << sw->getId() << "\n";
        printDiscreteMeasurements(*sw, prefix);
    }
}

} // namespace example
