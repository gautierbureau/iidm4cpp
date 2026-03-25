#include <gtest/gtest.h>
#include <iidm/Generator.h>
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
        backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_Q}]           = 30.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_V}]           = 1.05;
        backend.doubles[{GEN_HANDLE, prop::GEN_MIN_P}]              = 0.0;
        backend.doubles[{GEN_HANDLE, prop::GEN_MAX_P}]              = 300.0;
        backend.bools  [{GEN_HANDLE, prop::GEN_VOLTAGE_REGULATOR_ON}] = true;
        backend.ints   [{GEN_HANDLE, prop::GEN_ENERGY_SOURCE}]      = 0; // HYDRO
    }
};

TEST_F(GeneratorTest, GetId) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_EQ(gen.getId(), "GEN1");
}

TEST_F(GeneratorTest, GetName) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_EQ(gen.getName(), "Generator 1");
}

TEST_F(GeneratorTest, GetTargetP) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(gen.getTargetP(), 150.0);
}

TEST_F(GeneratorTest, SetTargetP) {
    Generator gen(GEN_HANDLE, &backend);
    gen.setTargetP(200.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_P}]), 200.0);
}

TEST_F(GeneratorTest, MethodChaining) {
    Generator gen(GEN_HANDLE, &backend);
    gen.setTargetP(200.0).setTargetQ(50.0).setVoltageRegulatorOn(false);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_P}]), 200.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::GEN_TARGET_Q}]), 50.0);
    EXPECT_FALSE((backend.bools[{GEN_HANDLE, prop::GEN_VOLTAGE_REGULATOR_ON}]));
}

TEST_F(GeneratorTest, GetMinMaxP) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(gen.getMinP(), 0.0);
    EXPECT_DOUBLE_EQ(gen.getMaxP(), 300.0);
}

TEST_F(GeneratorTest, IsVoltageRegulatorOn) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_TRUE(gen.isVoltageRegulatorOn());
}

TEST_F(GeneratorTest, GetEnergySource) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_EQ(gen.getEnergySource(), EnergySource::HYDRO);
}

TEST_F(GeneratorTest, IsValid) {
    Generator valid(GEN_HANDLE, &backend);
    Generator invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(GeneratorTest, Equality) {
    Generator gen1(GEN_HANDLE, &backend);
    Generator gen2(GEN_HANDLE, &backend);
    Generator gen3(99, &backend);
    EXPECT_EQ(gen1, gen2);
    EXPECT_NE(gen1, gen3);
}
