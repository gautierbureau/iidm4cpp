#include <gtest/gtest.h>
#include <iidm/Generator.h>
#include <iidm/Terminal.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"
#include <cmath>

using namespace iidm;
using namespace iidm::test;

class GeneratorTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle GEN_HANDLE = 42;

    void SetUp() override {
        backend.strings[{GEN_HANDLE, prop::ID}]   = "GEN1";
        backend.strings[{GEN_HANDLE, prop::NAME}]  = "Generator 1";
        backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_P}]           = 150.0;
        backend.doubles[{GEN_HANDLE, prop::LOCAL_TARGET_Q}]         = 30.0;
        backend.doubles[{GEN_HANDLE, prop::LOCAL_TARGET_V}]         = 405.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_MIN_P}]              = 0.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_MAX_P}]              = 300.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_RATED_S}]            = 350.0;
        backend.bools  [{GEN_HANDLE, prop::REGULATING}]             = true;
        backend.ints   [{GEN_HANDLE, prop::GEN_ENERGY_SOURCE}]      = 0; // HYDRO
    }
};

TEST_F(GeneratorTest, MethodChaining) {
    Generator gen(GEN_HANDLE, &backend);
    gen.setTargetP(200.0).setLocalTargetQ(50.0).setLocalTargetV(400.0).setMinP(10.0).setMaxP(250.0)
       .setRatedS(300.0).setEnergySource(EnergySource::WIND);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_P}]), 200.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::LOCAL_TARGET_Q}]), 50.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::LOCAL_TARGET_V}]), 400.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::GEN_MIN_P}]), 10.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::GEN_MAX_P}]), 250.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::GEN_RATED_S}]), 300.0);
    EXPECT_EQ((backend.ints[{GEN_HANDLE, prop::GEN_ENERGY_SOURCE}]), static_cast<int>(EnergySource::WIND));
}

TEST_F(GeneratorTest, LocalTargetsAndRatedS) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(gen.getLocalTargetQ(), 30.0);
    EXPECT_DOUBLE_EQ(gen.getLocalTargetV(), 405.0);
    EXPECT_DOUBLE_EQ(gen.getRatedS(), 350.0);
    EXPECT_TRUE(gen.isRegulating());
}

TEST_F(GeneratorTest, VoltageRegulation) {
    static constexpr ObjectHandle VR_H = 300;
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_FALSE(gen.getVoltageRegulation());

    backend.related[{GEN_HANDLE, prop::VOLTAGE_REGULATION}] = VR_H;
    backend.ints[{VR_H, prop::VR_MODE}] = static_cast<int>(RegulationMode::REACTIVE_POWER);
    backend.bools[{VR_H, prop::VR_REGULATING}] = true;
    auto regulation = gen.getVoltageRegulation();
    ASSERT_TRUE(regulation);
    EXPECT_EQ(*regulation->getMode(), RegulationMode::REACTIVE_POWER);
    EXPECT_TRUE(regulation->isRegulating());
    EXPECT_FALSE(regulation->getTerminal());

    regulation->setTargetDeadband(1.5).setSlope(0.2).setMode(RegulationMode::VOLTAGE);
    EXPECT_DOUBLE_EQ((backend.doubles[{VR_H, prop::VR_TARGET_DEADBAND}]), 1.5);
    EXPECT_DOUBLE_EQ((backend.doubles[{VR_H, prop::VR_SLOPE}]), 0.2);
    EXPECT_EQ((backend.ints[{VR_H, prop::VR_MODE}]), static_cast<int>(RegulationMode::VOLTAGE));

    // Undefined mode
    backend.ints[{VR_H, prop::VR_MODE}] = -1;
    EXPECT_FALSE(gen.getVoltageRegulation()->getMode());

    gen.removeVoltageRegulation();
    EXPECT_EQ((backend.related[{GEN_HANDLE, prop::VOLTAGE_REGULATION}]), INVALID_HANDLE);
}

// ── Generator::getRegulatingTerminal ─────────────────────────────────────────

TEST_F(GeneratorTest, GetRegulatingTerminal) {
    static constexpr ObjectHandle TERM_H = 200;
    backend.related[{GEN_HANDLE, prop::REL_REGULATING_TERMINAL}] = TERM_H;
    backend.doubles[{TERM_H, prop::TERMINAL_P}] = 75.0;
    Generator gen(GEN_HANDLE, &backend);
    Terminal t = gen.getRegulatingTerminal();
    EXPECT_TRUE(t.isValid());
    EXPECT_DOUBLE_EQ(t.getP(), 75.0);
}

TEST_F(GeneratorTest, SolvedValues) {
    Generator gen(GEN_HANDLE, &backend);
    gen.applySolvedValues();
    gen.setTargetPToP();
    ASSERT_EQ(backend.invocations.size(), 2u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(GEN_HANDLE), prop::ACT_APPLY_SOLVED_VALUES));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(GEN_HANDLE), prop::ACT_SET_TARGET_P_TO_P));
}
