#pragma once

#include <iidm/Adder.h>
#include <iidm/BoundaryLine.h>
#include <string>

namespace iidm {

// New boundary line of a voltage level (VoltageLevel::newBoundaryLine()): a line to the outside of the
// network, with a generation at its far end when given one (newGeneration())
class BoundaryLineAdder : public detail::InjectionAdder<BoundaryLineAdder> {
public:
    class GenerationAdder;

    BoundaryLineAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    BoundaryLineAdder& setP0(double p0) { return doubleField(prop::BL_P0, p0); }
    BoundaryLineAdder& setQ0(double q0) { return doubleField(prop::BL_Q0, q0); }
    BoundaryLineAdder& setR(double r)   { return doubleField(prop::BL_R, r); }
    BoundaryLineAdder& setX(double x)   { return doubleField(prop::BL_X, x); }
    BoundaryLineAdder& setG(double g)   { return doubleField(prop::BL_G, g); }
    BoundaryLineAdder& setB(double b)   { return doubleField(prop::BL_B, b); }
    // The key that pairs it with a boundary line of another network into a tie line
    BoundaryLineAdder& setPairingKey(const std::string& pairingKey) { return stringField(prop::BL_PAIRING_KEY, pairingKey); }

    GenerationAdder newGeneration();

    BoundaryLine add() { return BoundaryLine(create(), backend_); }
};

// The generation of a new boundary line: add() returns to the line's adder
class BoundaryLineAdder::GenerationAdder : public detail::AdderBase<GenerationAdder> {
public:
    GenerationAdder(ObjectHandle adder, BackendProvider* backend, BoundaryLineAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    GenerationAdder& setTargetP(double targetP) { return doubleField(prop::GENERATION_TARGET_P, targetP); }
    GenerationAdder& setMinP(double minP)       { return doubleField(prop::GENERATION_MIN_P, minP); }
    GenerationAdder& setMaxP(double maxP)       { return doubleField(prop::GENERATION_MAX_P, maxP); }
    GenerationAdder& setTargetQ(double targetQ) { return doubleField(prop::GENERATION_TARGET_Q, targetQ); }
    GenerationAdder& setTargetV(double targetV) { return doubleField(prop::GENERATION_TARGET_V, targetV); }
    GenerationAdder& setVoltageRegulationOn(bool voltageRegulationOn) {
        return boolField(prop::GENERATION_VOLTAGE_REGULATION_ON, voltageRegulationOn);
    }

    BoundaryLineAdder& add() {
        create();
        return parent_;
    }

private:
    BoundaryLineAdder& parent_;
};

inline BoundaryLineAdder::GenerationAdder BoundaryLineAdder::newGeneration() {
    return GenerationAdder(backend_->invoke(adder_, prop::BOUNDARY_LINE_GENERATION), backend_, *this);
}

} // namespace iidm
