#include <gtest/gtest.h>
#include <iidm/Generator.h>
#include <iidm/Line.h>
#include <iidm/Load.h>
#include <iidm/Switch.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class MeasurementsTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle GEN_H = 50;
    static constexpr ObjectHandle M1_H  = 51;
    static constexpr ObjectHandle M2_H  = 52;

    void SetUp() override {
        backend.bools[{GEN_H, prop::EXT_MEAS_EXISTS}] = true;
        backend.children[{GEN_H, prop::MEASUREMENT}] = {M1_H, M2_H};
        backend.relatedByString[std::make_tuple(GEN_H, prop::REL_MEASUREMENT_BY_ID, std::string("G_P"))] = M1_H;

        backend.strings[{M1_H, prop::MEAS_ID}]      = "G_P";
        backend.ints   [{M1_H, prop::MEAS_TYPE}]    = static_cast<int>(MeasurementType::ACTIVE_POWER);
        backend.ints   [{M1_H, prop::MEAS_SIDE}]    = -1;
        backend.doubles[{M1_H, prop::MEAS_VALUE}]   = 101.5;
        backend.doubles[{M1_H, prop::MEAS_STD_DEV}] = 1.0;
        backend.bools  [{M1_H, prop::MEAS_VALID}]   = true;

        backend.strings[{M2_H, prop::MEAS_ID}]   = "G_Q";
        backend.ints   [{M2_H, prop::MEAS_TYPE}] = static_cast<int>(MeasurementType::REACTIVE_POWER);
        backend.ints   [{M2_H, prop::MEAS_SIDE}] = static_cast<int>(ThreeSides::TWO);
    }
};

TEST_F(MeasurementsTest, HasExtensionOnConnectables) {
    EXPECT_TRUE(Generator(GEN_H, &backend).hasMeasurements());
    MockBackend empty;
    EXPECT_FALSE(Load(GEN_H, &empty).hasMeasurements());
    EXPECT_FALSE(Line(GEN_H, &empty).hasMeasurements());
    EXPECT_FALSE(TwoWindingsTransformer(GEN_H, &empty).hasMeasurements());
    EXPECT_FALSE(ThreeWindingsTransformer(GEN_H, &empty).hasMeasurements());
}

TEST_F(MeasurementsTest, ListAndFilterByType) {
    auto meas = Generator(GEN_H, &backend).getMeasurements();
    auto all = meas.getMeasurements();
    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].getId(), "G_P");
    EXPECT_EQ(all[1].getId(), "G_Q");
    auto reactive = meas.getMeasurements(MeasurementType::REACTIVE_POWER);
    ASSERT_EQ(reactive.size(), 1u);
    EXPECT_EQ(reactive[0].getId(), "G_Q");
    EXPECT_TRUE(meas.getMeasurements(MeasurementType::VOLTAGE).empty());
}

TEST_F(MeasurementsTest, LookupById) {
    auto meas = Generator(GEN_H, &backend).getMeasurements();
    auto p = meas.getMeasurement("G_P");
    ASSERT_TRUE(p.has_value());
    EXPECT_EQ(p->getType(), MeasurementType::ACTIVE_POWER);
    EXPECT_FALSE(meas.getMeasurement("UNKNOWN").has_value());
}

TEST_F(MeasurementsTest, MeasurementProperties) {
    Measurement p(M1_H, &backend);
    EXPECT_TRUE(p.isValid());
    EXPECT_DOUBLE_EQ(p.getValue(), 101.5);
    EXPECT_DOUBLE_EQ(p.getStandardDeviation(), 1.0);
    EXPECT_TRUE(p.isValueValid());
    EXPECT_FALSE(p.getSide().has_value());

    Measurement q(M2_H, &backend);
    ASSERT_TRUE(q.getSide().has_value());
    EXPECT_EQ(*q.getSide(), ThreeSides::TWO);
}

TEST_F(MeasurementsTest, MeasurementSetters) {
    Measurement p(M1_H, &backend);
    p.setValue(99.0).setStandardDeviation(2.5).setValueValid(false);
    EXPECT_DOUBLE_EQ((backend.doubles[{M1_H, prop::MEAS_VALUE}]), 99.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{M1_H, prop::MEAS_STD_DEV}]), 2.5);
    EXPECT_FALSE((backend.bools[{M1_H, prop::MEAS_VALID}]));
    p.setValueAndValidity(98.0, true);
    EXPECT_DOUBLE_EQ((backend.doubles[{M1_H, prop::MEAS_VALUE}]), 98.0);
    EXPECT_TRUE((backend.bools[{M1_H, prop::MEAS_VALID}]));
}

class DiscreteMeasurementsTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle SW_H  = 60;
    static constexpr ObjectHandle TAP_H = 61;
    static constexpr ObjectHandle POS_H = 62;

    void SetUp() override {
        backend.bools[{SW_H, prop::EXT_DMEAS_EXISTS}] = true;
        backend.children[{SW_H, prop::DISCRETE_MEASUREMENT}] = {TAP_H, POS_H};
        backend.relatedByString[std::make_tuple(SW_H, prop::REL_DISCRETE_MEASUREMENT_BY_ID, std::string("POS"))] = POS_H;

        backend.strings[{TAP_H, prop::DMEAS_ID}]          = "TAP";
        backend.ints   [{TAP_H, prop::DMEAS_TYPE}]        = static_cast<int>(DiscreteMeasurementType::TAP_POSITION);
        backend.ints   [{TAP_H, prop::DMEAS_TAP_CHANGER}] = static_cast<int>(DiscreteMeasurementTapChanger::PHASE_TAP_CHANGER_2);
        backend.ints   [{TAP_H, prop::DMEAS_VALUE_TYPE}]  = static_cast<int>(DiscreteMeasurementValueType::INT);
        backend.ints   [{TAP_H, prop::DMEAS_VALUE_INT}]   = 3;

        backend.strings[{POS_H, prop::DMEAS_ID}]          = "POS";
        backend.ints   [{POS_H, prop::DMEAS_TYPE}]        = static_cast<int>(DiscreteMeasurementType::SWITCH_POSITION);
        backend.ints   [{POS_H, prop::DMEAS_TAP_CHANGER}] = -1;
        backend.ints   [{POS_H, prop::DMEAS_VALUE_TYPE}]  = static_cast<int>(DiscreteMeasurementValueType::BOOLEAN);
        backend.bools  [{POS_H, prop::DMEAS_VALUE_BOOL}]  = true;
        backend.bools  [{POS_H, prop::DMEAS_VALID}]       = true;
    }
};

TEST_F(DiscreteMeasurementsTest, Hosts) {
    EXPECT_TRUE(Switch(SW_H, &backend).hasDiscreteMeasurements());
    MockBackend empty;
    EXPECT_FALSE(TwoWindingsTransformer(SW_H, &empty).hasDiscreteMeasurements());
    EXPECT_FALSE(ThreeWindingsTransformer(SW_H, &empty).hasDiscreteMeasurements());
}

TEST_F(DiscreteMeasurementsTest, ListFilterAndLookup) {
    auto ext = Switch(SW_H, &backend).getDiscreteMeasurements();
    EXPECT_EQ(ext.getDiscreteMeasurements().size(), 2u);
    auto taps = ext.getDiscreteMeasurements(DiscreteMeasurementType::TAP_POSITION);
    ASSERT_EQ(taps.size(), 1u);
    EXPECT_EQ(taps[0].getId(), "TAP");
    auto pos = ext.getDiscreteMeasurement("POS");
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(pos->getType(), DiscreteMeasurementType::SWITCH_POSITION);
    EXPECT_FALSE(ext.getDiscreteMeasurement("UNKNOWN").has_value());
}

TEST_F(DiscreteMeasurementsTest, Values) {
    DiscreteMeasurement tap(TAP_H, &backend);
    ASSERT_TRUE(tap.getTapChanger().has_value());
    EXPECT_EQ(*tap.getTapChanger(), DiscreteMeasurementTapChanger::PHASE_TAP_CHANGER_2);
    EXPECT_EQ(tap.getValueType(), DiscreteMeasurementValueType::INT);
    EXPECT_EQ(tap.getValueAsInt(), 3);

    DiscreteMeasurement pos(POS_H, &backend);
    EXPECT_FALSE(pos.getTapChanger().has_value());
    EXPECT_TRUE(pos.getValueAsBoolean());
    EXPECT_TRUE(pos.isValueValid());
}

TEST_F(DiscreteMeasurementsTest, SetValueOverloads) {
    DiscreteMeasurement m(TAP_H, &backend);
    m.setValue(5);
    EXPECT_EQ((backend.ints[{TAP_H, prop::DMEAS_VALUE_INT}]), 5);
    m.setValue(false);
    EXPECT_FALSE((backend.bools[{TAP_H, prop::DMEAS_VALUE_BOOL}]));
    m.setValue("OPEN");   // must not decay to the bool overload
    EXPECT_EQ((backend.strings[{TAP_H, prop::DMEAS_VALUE_STRING}]), "OPEN");
    m.setValue(std::string("CLOSED")).setValueValid(false);
    EXPECT_EQ((backend.strings[{TAP_H, prop::DMEAS_VALUE_STRING}]), "CLOSED");
    EXPECT_FALSE((backend.bools[{TAP_H, prop::DMEAS_VALID}]));
}
