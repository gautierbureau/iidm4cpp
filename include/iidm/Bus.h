#pragma once

#include <iidm/Component.h>
#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
#include <iidm/stdcxx/optional.hpp>
#include <vector>

namespace iidm {

class VoltageLevel;
class Terminal;
class Line;
class TwoWindingsTransformer;
class ThreeWindingsTransformer;
class Generator;
class Battery;
class Load;
class ShuntCompensator;
class BoundaryLine;
class StaticVarCompensator;
class LccConverterStation;
class VscConverterStation;

// A bus of a bus-breaker view (a configured bus, or one made of nodes) or of a bus view
class Bus : public Identifiable {
public:
    static constexpr int TYPE = prop::BUS;

    Bus() = default;
    explicit Bus(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    double getV() const { return getDouble(prop::BUS_V); }
    Bus& setV(double v) { return setDouble(*this, prop::BUS_V, v); }

    double getAngle() const { return getDouble(prop::BUS_ANGLE); }
    Bus& setAngle(double angle) { return setDouble(*this, prop::BUS_ANGLE, angle); }

    // Unsets its voltage and angle
    void unsetSolvedValues();

    // Active (reactive) power of its equipment, load sign convention; NaN when nothing is connected
    double getP() const { return getDouble(prop::BUS_P); }
    double getQ() const { return getDouble(prop::BUS_Q); }

    // Fictitious injection, load sign convention (0 when none)
    double getFictitiousP0() const { return getDouble(prop::FICTITIOUS_P0); }
    Bus& setFictitiousP0(double p0) { return setDouble(*this, prop::FICTITIOUS_P0, p0); }
    double getFictitiousQ0() const { return getDouble(prop::FICTITIOUS_Q0); }
    Bus& setFictitiousQ0(double q0) { return setDouble(*this, prop::FICTITIOUS_Q0, q0); }

    // Its components (those of its bus view bus); empty when it is in none (nothing connected)
    iidm::optional<Component> getConnectedComponent() const;
    iidm::optional<Component> getSynchronousComponent() const;
    bool isInMainConnectedComponent() const { return getBool(prop::BUS_IN_MAIN_CONNECTED_COMPONENT); }
    bool isInMainSynchronousComponent() const { return getBool(prop::BUS_IN_MAIN_SYNCHRONOUS_COMPONENT); }

    VoltageLevel getVoltageLevel() const;

    // ── What is connected to it (include the headers of the types used) ──────
    std::vector<Terminal> getConnectedTerminals() const;
    int getConnectedTerminalCount() const;
    std::vector<Line>                     getLines() const;
    std::vector<TwoWindingsTransformer>   getTwoWindingsTransformers() const;
    std::vector<ThreeWindingsTransformer> getThreeWindingsTransformers() const;
    std::vector<Generator>                getGenerators() const;
    std::vector<Battery>                  getBatteries() const;
    std::vector<Load>                     getLoads() const;
    std::vector<ShuntCompensator>         getShuntCompensators() const;
    std::vector<BoundaryLine>             getBoundaryLines(BoundaryLineFilter filter = BoundaryLineFilter::ALL) const;
    std::vector<StaticVarCompensator>     getStaticVarCompensators() const;
    std::vector<LccConverterStation>      getLccConverterStations() const;
    std::vector<VscConverterStation>      getVscConverterStations() const;
};

} // namespace iidm
