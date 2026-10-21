#include <gtest/gtest.h>
#include <iidm/Generator.h>
#include <iidm/Line.h>
#include <iidm/Load.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/TwoWindingsTransformer.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

// The Fortescue extensions, the asymmetrical load and the phase angle clocks as the backend sees
// them: the extension related to its host by its type code, its properties (enums as ordinals); the
// legs of a three windings transformer's by ThreeSides ordinal, its adder selecting a leg before the
// leg fields.

static constexpr ObjectHandle HOST_HANDLE = 10;
static constexpr ObjectHandle EXT_HANDLE = 20;
static constexpr ObjectHandle LEG_HANDLE = 21;
static constexpr ObjectHandle ADDER_HANDLE = 500; // the next "adder" of the mock

class AsymmetricalExtensionsTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.nextAdder = ADDER_HANDLE;
    }

    void attach(int type) {
        backend.related[{HOST_HANDLE, type}] = EXT_HANDLE;
    }
};

TEST_F(AsymmetricalExtensionsTest, GeneratorAndLine) {
    attach(prop::GENERATOR_FORTESCUE);
    backend.bools[{EXT_HANDLE, prop::EXT_GF_GROUNDED}] = true;
    GeneratorFortescue gf = Generator(HOST_HANDLE, &backend).getGeneratorFortescue();
    EXPECT_TRUE(gf.isGrounded());
    gf.setRz(0.1).setXz(2).setRn(0.2).setXn(3).setGroundingR(0.5).setGroundingX(1.5);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_GF_XN}]), 3.0);
    EXPECT_DOUBLE_EQ(gf.getGroundingX(), 1.5);

    attach(prop::LINE_FORTESCUE);
    LineFortescue lf = Line(HOST_HANDLE, &backend).getLineFortescue();
    lf.setOpenPhaseB(true).setG2z(0.002).setB1z(0.0001);
    EXPECT_TRUE(lf.isOpenPhaseB());
    EXPECT_FALSE(lf.isOpenPhaseA());
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_LINE_F_G2Z}]), 0.002);
    Line(HOST_HANDLE, &backend).newLineFortescue().withRz(3).withOpenPhaseC(true);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(HOST_HANDLE), prop::LINE_FORTESCUE));
    EXPECT_TRUE((backend.bools[{ADDER_HANDLE, prop::EXT_LINE_F_OPEN_PHASE_C}]));
}

TEST_F(AsymmetricalExtensionsTest, Load) {
    attach(prop::LOAD_ASYMMETRICAL);
    backend.ints[{EXT_HANDLE, prop::EXT_LA_CONNECTION_TYPE}] = static_cast<int>(LoadConnectionType::DELTA);
    LoadAsymmetrical la = Load(HOST_HANDLE, &backend).getLoadAsymmetrical();
    EXPECT_EQ(la.getConnectionType(), LoadConnectionType::DELTA);
    la.setConnectionType(LoadConnectionType::Y).setDeltaPc(3).setDeltaQb(0.2);
    EXPECT_EQ((backend.ints[{EXT_HANDLE, prop::EXT_LA_CONNECTION_TYPE}]), 0);
    EXPECT_DOUBLE_EQ(la.getDeltaPc(), 3.0);
    Load(HOST_HANDLE, &backend).newLoadAsymmetrical().withConnectionType(LoadConnectionType::DELTA).withDeltaQa(0.1);
    EXPECT_EQ((backend.ints[{ADDER_HANDLE, prop::EXT_LA_CONNECTION_TYPE}]), 1);
}

TEST_F(AsymmetricalExtensionsTest, TwoWindingsTransformer) {
    attach(prop::TWO_WINDINGS_TRANSFORMER_FORTESCUE);
    TwoWindingsTransformerFortescue tf = TwoWindingsTransformer(HOST_HANDLE, &backend).getTwoWindingsTransformerFortescue();
    tf.setConnectionType1(WindingConnectionType::Y_GROUNDED).setConnectionType2(WindingConnectionType::DELTA)
      .setFreeFluxes(true).setXm(500).setGroundingX2(0.04);
    EXPECT_EQ((backend.ints[{EXT_HANDLE, prop::EXT_TWO_WTF_CONNECTION_TYPE1}]), 1);
    EXPECT_EQ(tf.getConnectionType2(), WindingConnectionType::DELTA);
    EXPECT_TRUE(tf.isFreeFluxes());
    EXPECT_DOUBLE_EQ(tf.getGroundingX2(), 0.04);

    attach(prop::TWO_WINDINGS_TRANSFORMER_PHASE_ANGLE_CLOCK);
    backend.ints[{EXT_HANDLE, prop::EXT_TWO_WT_PAC_CLOCK}] = 11;
    TwoWindingsTransformer t2(HOST_HANDLE, &backend);
    EXPECT_EQ(t2.getTwoWindingsTransformerPhaseAngleClock().getPhaseAngleClock(), 11);
    t2.newTwoWindingsTransformerPhaseAngleClock().withPhaseAngleClock(5);
    EXPECT_EQ((backend.ints[{ADDER_HANDLE, prop::EXT_TWO_WT_PAC_CLOCK}]), 5);
}

TEST_F(AsymmetricalExtensionsTest, ThreeWindingsTransformer) {
    attach(prop::THREE_WINDINGS_TRANSFORMER_FORTESCUE);
    backend.relatedByIndex[std::make_tuple(ObjectHandle(EXT_HANDLE), prop::LEG_FORTESCUE, 1)] = LEG_HANDLE;
    backend.doubles[{LEG_HANDLE, prop::EXT_LEG_F_XZ}] = 22.0;
    ThreeWindingsTransformer t3(HOST_HANDLE, &backend);
    LegFortescue leg2 = t3.getThreeWindingsTransformerFortescue().getLeg2();
    EXPECT_DOUBLE_EQ(leg2.getXz(), 22.0);
    leg2.setConnectionType(WindingConnectionType::Y).setFreeFluxes(true).setGroundingR(0.1);
    EXPECT_EQ((backend.ints[{LEG_HANDLE, prop::EXT_LEG_F_CONNECTION_TYPE}]), 0);
    EXPECT_TRUE(leg2.isFreeFluxes());
    EXPECT_FALSE(t3.getThreeWindingsTransformerFortescue().getLeg1().isValid());

    // Its adder: the leg selected, then its fields
    t3.newThreeWindingsTransformerFortescue().leg1().withRz(1).leg3().withXz(3).withConnectionType(WindingConnectionType::DELTA);
    EXPECT_EQ((backend.ints[{ADDER_HANDLE, prop::EXT_THREE_WTF_LEG}]), 2); // the last one selected
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE, prop::EXT_LEG_F_RZ}]), 1.0);
    EXPECT_EQ((backend.ints[{ADDER_HANDLE, prop::EXT_LEG_F_CONNECTION_TYPE}]), 2);

    attach(prop::THREE_WINDINGS_TRANSFORMER_PHASE_ANGLE_CLOCK);
    ThreeWindingsTransformerPhaseAngleClock clock = t3.getThreeWindingsTransformerPhaseAngleClock();
    clock.setPhaseAngleClockLeg2(5).setPhaseAngleClockLeg3(7);
    EXPECT_EQ(clock.getPhaseAngleClockLeg2(), 5);
    EXPECT_EQ((backend.ints[{EXT_HANDLE, prop::EXT_THREE_WT_PAC_CLOCK_LEG3}]), 7);
}
