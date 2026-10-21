#pragma once

#include <iidm/FlowsLimitsHolder.h>
#include <iidm/Injection.h>
#include <iidm/ReactiveLimitsHolder.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {

class Boundary;
class VoltageLevel;

// A line from the network to its outside (powsybl 7.4 BoundaryLine; DanglingLine before), which
// ends at its boundary. Two of them, paired by their pairing key, can make a tie line.
class BoundaryLine : public Injection, public detail::FlowsLimitsHolderMethods<BoundaryLine> {
public:
    // The generation at the boundary, when it has one
    class Generation : public detail::ObjectRef, public detail::ReactiveLimitsHolderMethods<Generation> {
    public:
        static constexpr int TYPE = prop::BOUNDARY_LINE_GENERATION;

        Generation() = default;
        explicit Generation(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

        double getTargetP() const;
        Generation& setTargetP(double targetP);
        double getMinP() const;
        Generation& setMinP(double minP);
        double getMaxP() const;
        Generation& setMaxP(double maxP);
        double getTargetQ() const;
        Generation& setTargetQ(double targetQ);
        bool isVoltageRegulationOn() const;
        Generation& setVoltageRegulationOn(bool voltageRegulationOn);
        double getTargetV() const;
        Generation& setTargetV(double targetV);
    };

    static constexpr int TYPE = prop::BOUNDARY_LINE;

    BoundaryLine() = default;
    explicit BoundaryLine(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getP0() const;
    BoundaryLine& setP0(double p0);
    double getQ0() const;
    BoundaryLine& setQ0(double q0);

    double getR() const;
    BoundaryLine& setR(double r);
    double getX() const;
    BoundaryLine& setX(double x);
    double getG() const;
    BoundaryLine& setG(double g);
    double getB() const;
    BoundaryLine& setB(double b);

    // In a tie line
    bool isPaired() const;
    // Empty when it has none
    iidm::optional<std::string> getPairingKey() const;
    BoundaryLine& setPairingKey(const std::string& pairingKey);

    // Empty when it has none
    iidm::optional<Generation> getGeneration() const;
    Boundary getBoundary() const;

    // ── Solved values ────────────────────────────────────────────────────
    // The targets of its generation from the solved flows and voltage: all of them, then each
    void applySolvedValues();
    void setGenerationTargetPToP();
    void setGenerationTargetQToQ();
    void setGenerationTargetVToV();
};

// The far end of a boundary line (powsybl Boundary): the voltage and the flows there, computed
// from those of the network side
class Boundary : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::BOUNDARY;

    Boundary() = default;
    explicit Boundary(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    double getV() const;
    double getAngle() const;
    double getP() const;
    double getQ() const;
    double getI() const;

    BoundaryLine getBoundaryLine() const;
    VoltageLevel getNetworkSideVoltageLevel() const;
};

} // namespace iidm
