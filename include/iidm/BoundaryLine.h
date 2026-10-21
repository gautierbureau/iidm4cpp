#pragma once

#include <iidm/FlowsLimitsHolder.h>
#include <iidm/Injection.h>
#include <iidm/ReactiveLimitsHolder.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {

class Boundary;
class TieLine;
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

        double getTargetP() const { return getDouble(prop::GENERATION_TARGET_P); }
        Generation& setTargetP(double targetP) { return setDouble(*this, prop::GENERATION_TARGET_P, targetP); }
        double getMinP() const { return getDouble(prop::GENERATION_MIN_P); }
        Generation& setMinP(double minP) { return setDouble(*this, prop::GENERATION_MIN_P, minP); }
        double getMaxP() const { return getDouble(prop::GENERATION_MAX_P); }
        Generation& setMaxP(double maxP) { return setDouble(*this, prop::GENERATION_MAX_P, maxP); }
        double getTargetQ() const { return getDouble(prop::GENERATION_TARGET_Q); }
        Generation& setTargetQ(double targetQ) { return setDouble(*this, prop::GENERATION_TARGET_Q, targetQ); }
        bool isVoltageRegulationOn() const { return getBool(prop::GENERATION_VOLTAGE_REGULATION_ON); }
        Generation& setVoltageRegulationOn(bool voltageRegulationOn) {
            return setBool(*this, prop::GENERATION_VOLTAGE_REGULATION_ON, voltageRegulationOn);
        }
        double getTargetV() const { return getDouble(prop::GENERATION_TARGET_V); }
        Generation& setTargetV(double targetV) { return setDouble(*this, prop::GENERATION_TARGET_V, targetV); }
    };

    static constexpr int TYPE = prop::BOUNDARY_LINE;

    BoundaryLine() = default;
    explicit BoundaryLine(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getP0() const { return getDouble(prop::BL_P0); }
    BoundaryLine& setP0(double p0) { return setDouble(*this, prop::BL_P0, p0); }
    double getQ0() const { return getDouble(prop::BL_Q0); }
    BoundaryLine& setQ0(double q0) { return setDouble(*this, prop::BL_Q0, q0); }

    double getR() const { return getDouble(prop::BL_R); }
    BoundaryLine& setR(double r) { return setDouble(*this, prop::BL_R, r); }
    double getX() const { return getDouble(prop::BL_X); }
    BoundaryLine& setX(double x) { return setDouble(*this, prop::BL_X, x); }
    double getG() const { return getDouble(prop::BL_G); }
    BoundaryLine& setG(double g) { return setDouble(*this, prop::BL_G, g); }
    double getB() const { return getDouble(prop::BL_B); }
    BoundaryLine& setB(double b) { return setDouble(*this, prop::BL_B, b); }

    // In a tie line; that tie line (empty when unpaired; include <iidm/TieLine.h> to use it)
    bool isPaired() const { return getBool(prop::BL_PAIRED); }
    iidm::optional<TieLine> getTieLine() const;
    // Empty when it has none
    iidm::optional<std::string> getPairingKey() const { return getOptString(prop::BL_PAIRING_KEY); }
    BoundaryLine& setPairingKey(const std::string& pairingKey) {
        return setString(*this, prop::BL_PAIRING_KEY, pairingKey);
    }

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

    double getV() const { return getDouble(prop::BOUNDARY_V); }
    double getAngle() const { return getDouble(prop::BOUNDARY_ANGLE); }
    double getP() const { return getDouble(prop::BOUNDARY_P); }
    double getQ() const { return getDouble(prop::BOUNDARY_Q); }
    double getI() const { return getDouble(prop::BOUNDARY_I); }

    BoundaryLine getBoundaryLine() const;
    VoltageLevel getNetworkSideVoltageLevel() const;
};

} // namespace iidm
