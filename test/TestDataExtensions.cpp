#include <gtest/gtest.h>
#include <iidm/Battery.h>
#include <iidm/BatteryShortCircuitAdder.h>
#include <iidm/DynamicModelInfo.h>
#include <iidm/DynamicModelInfoAdder.h>
#include <iidm/Generator.h>
#include <iidm/GeneratorEntsoeCategory.h>
#include <iidm/GeneratorEntsoeCategoryAdder.h>
#include <iidm/GeneratorShortCircuit.h>
#include <iidm/GeneratorStartup.h>
#include <iidm/GeneratorStartupAdder.h>
#include <iidm/IdentifiableShortCircuit.h>
#include <iidm/Load.h>
#include <iidm/LoadDetail.h>
#include <iidm/ManualFrequencyRestorationReserve.h>
#include <iidm/OperatingStatus.h>
#include <iidm/OperatingStatusAdder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/StandbyAutomaton.h>
#include <iidm/StandbyAutomatonAdder.h>
#include <iidm/StaticVarCompensator.h>
#include "MockBackend.h"
#include <string>

using namespace iidm;
using namespace iidm::test;

// The data extensions as the backend sees them: each one related to its host by its type code,
// its properties read and written with their codes, a new one started on the host by the type code,
// removed by clearing it.

static constexpr ObjectHandle GEN_HANDLE = 10;
static constexpr ObjectHandle BAT_HANDLE = 11;
static constexpr ObjectHandle LOAD_HANDLE = 12;
static constexpr ObjectHandle SVC_HANDLE = 13;
static constexpr ObjectHandle LINE_HANDLE = 14;
static constexpr ObjectHandle VL_HANDLE = 15;
static constexpr ObjectHandle EXT_HANDLE = 20; // the extension object
static constexpr ObjectHandle ADDER_HANDLE = 500; // the next "adder" of the mock

class DataExtensionsTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.nextAdder = ADDER_HANDLE;
        backend.strings[{GEN_HANDLE, prop::ID}] = "G";
    }

    // The host has the extension of that type, EXT_HANDLE
    void attach(ObjectHandle host, int type) {
        backend.related[{host, type}] = EXT_HANDLE;
    }
};

TEST_F(DataExtensionsTest, Accessors) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_FALSE(gen.hasExtension<GeneratorStartup>());
    EXPECT_THROW(gen.getExtension<GeneratorStartup>(), ExtensionNotFoundException);
    attach(GEN_HANDLE, prop::GENERATOR_STARTUP);
    EXPECT_TRUE(gen.hasExtension<GeneratorStartup>());
    EXPECT_EQ(detail::Access::handle(gen.getExtension<GeneratorStartup>()), ObjectHandle(EXT_HANDLE));

    gen.removeExtension<GeneratorStartup>();
    EXPECT_EQ((backend.related[{GEN_HANDLE, prop::GENERATOR_STARTUP}]), INVALID_HANDLE);
    backend.addResults[ADDER_HANDLE] = EXT_HANDLE;
    GeneratorStartup added = gen.newExtension<GeneratorStartupAdder>().withStartupCost(1000).add();
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(GEN_HANDLE), prop::GENERATOR_STARTUP));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE, prop::EXT_GS_STARTUP_COST}]), 1000.0);
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::ACT_ADD));
    EXPECT_EQ(detail::Access::handle(added), ObjectHandle(EXT_HANDLE));
}

TEST_F(DataExtensionsTest, ShortCircuits) {
    attach(GEN_HANDLE, prop::GENERATOR_SHORT_CIRCUIT);
    backend.doubles[{EXT_HANDLE, prop::EXT_SC_DIRECT_SUBTRANS_X}] = 0.2;
    GeneratorShortCircuit sc = Generator(GEN_HANDLE, &backend).getExtension<GeneratorShortCircuit>();
    EXPECT_DOUBLE_EQ(sc.getDirectSubtransX(), 0.2);
    sc.setDirectTransX(0.3).setStepUpTransformerX(0.1);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_SC_DIRECT_TRANS_X}]), 0.3);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_SC_STEP_UP_TRANSFORMER_X}]), 0.1);

    // A battery's: the same properties, another type
    Battery bat(BAT_HANDLE, &backend);
    bat.newExtension<BatteryShortCircuitAdder>().withDirectSubtransX(0.4).withDirectTransX(0.5).withStepUpTransformerX(0.6);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(BAT_HANDLE), prop::BATTERY_SHORT_CIRCUIT));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE, prop::EXT_SC_DIRECT_SUBTRANS_X}]), 0.4);
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE, prop::EXT_SC_STEP_UP_TRANSFORMER_X}]), 0.6);

    // An identifiable's (a voltage level's)
    attach(VL_HANDLE, prop::IDENTIFIABLE_SHORT_CIRCUIT);
    backend.doubles[{EXT_HANDLE, prop::EXT_ISC_IP_MAX}] = 5000.0;
    IdentifiableShortCircuit isc = Identifiable(VL_HANDLE, &backend).getExtension<IdentifiableShortCircuit>();
    EXPECT_DOUBLE_EQ(isc.getIpMax(), 5000.0);
    isc.setIpMin(1000).setIpMax(6000);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_ISC_IP_MIN}]), 1000.0);
}

TEST_F(DataExtensionsTest, LoadAndCompensatorData) {
    attach(LOAD_HANDLE, prop::LOAD_DETAIL);
    LoadDetail detail = Load(LOAD_HANDLE, &backend).getExtension<LoadDetail>();
    detail.setFixedActivePower(30).setFixedReactivePower(4).setVariableActivePower(50).setVariableReactivePower(6);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_LD_VARIABLE_ACTIVE_POWER}]), 50.0);
    EXPECT_DOUBLE_EQ(detail.getFixedReactivePower(), 4.0);

    StaticVarCompensator svc(SVC_HANDLE, &backend);
    svc.newExtension<StandbyAutomatonAdder>().withStandbyStatus(true).withB0(0.001).withLowVoltageSetpoint(390)
        .withHighVoltageSetpoint(410).withLowVoltageThreshold(380).withHighVoltageThreshold(420);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(SVC_HANDLE), prop::STANDBY_AUTOMATON));
    EXPECT_TRUE((backend.bools[{ADDER_HANDLE, prop::EXT_SA_STANDBY}]));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE, prop::EXT_SA_HIGH_VOLTAGE_THRESHOLD}]), 420.0);
    attach(SVC_HANDLE, prop::STANDBY_AUTOMATON);
    StandbyAutomaton automaton = svc.getExtension<StandbyAutomaton>();
    automaton.setStandby(false).setB0(0.002).setLowVoltageThreshold(370);
    EXPECT_FALSE(automaton.isStandby());
    EXPECT_DOUBLE_EQ(automaton.getB0(), 0.002);
    EXPECT_DOUBLE_EQ(automaton.getLowVoltageThreshold(), 370.0);
}

