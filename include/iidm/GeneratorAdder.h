#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/Generator.h>
#include <iidm/VoltageRegulationAdder.h>

namespace iidm {

// New generator of a voltage level (VoltageLevel::newGenerator())
class GeneratorAdder : public detail::InjectionAdder<GeneratorAdder> {
public:
    GeneratorAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    GeneratorAdder& setEnergySource(EnergySource energySource) { return intField(prop::GEN_ENERGY_SOURCE, static_cast<int>(energySource)); }
    GeneratorAdder& setMinP(double minP)                       { return doubleField(prop::GEN_MIN_P, minP); }
    GeneratorAdder& setMaxP(double maxP)                       { return doubleField(prop::GEN_MAX_P, maxP); }
    GeneratorAdder& setTargetP(double targetP)                 { return doubleField(prop::GEN_TARGET_P, targetP); }
    GeneratorAdder& setRatedS(double ratedS)                   { return doubleField(prop::GEN_RATED_S, ratedS); }
    GeneratorAdder& setCondenser(bool condenser)               { return boolField(prop::GEN_CONDENSER, condenser); }
    GeneratorAdder& setLocalTargetV(double localTargetV)       { return doubleField(prop::LOCAL_TARGET_V, localTargetV); }
    GeneratorAdder& setLocalTargetQ(double localTargetQ)       { return doubleField(prop::LOCAL_TARGET_Q, localTargetQ); }

    VoltageRegulationAdder<GeneratorAdder> newVoltageRegulation() {
        return VoltageRegulationAdder<GeneratorAdder>(backend_->invoke(adder_, prop::VOLTAGE_REGULATION), backend_, *this);
    }

    Generator add() { return Generator(create(), backend_); }
};

} // namespace iidm
