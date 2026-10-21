#pragma once

#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
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

    double getV() const;
    Bus& setV(double v);

    double getAngle() const;
    Bus& setAngle(double angle);

    // Unsets its voltage and angle
    void unsetSolvedValues();

    // Active (reactive) power of its equipment, load sign convention; NaN when nothing is connected
    double getP() const;
    double getQ() const;

    // Fictitious injection, load sign convention (0 when none)
    double getFictitiousP0() const;
    Bus& setFictitiousP0(double p0);
    double getFictitiousQ0() const;
    Bus& setFictitiousQ0(double q0);

    bool isInMainConnectedComponent() const;
    bool isInMainSynchronousComponent() const;

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
