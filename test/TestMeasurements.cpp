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
    static constexpr ObjectHandle EXT_H = 53; // the extension object
    static constexpr ObjectHandle M1_H  = 51;
    static constexpr ObjectHandle M2_H  = 52;

    void SetUp() override {
        backend.related[{GEN_H, prop::MEASUREMENTS}] = EXT_H;
        backend.children[{EXT_H, prop::MEASUREMENT}] = {M1_H, M2_H};
        backend.relatedByString[std::make_tuple(EXT_H, prop::MEASUREMENT, std::string("G_P"))] = M1_H;

        backend.optStrings[{M1_H, prop::MEAS_ID}]   = "G_P";
        backend.ints   [{M1_H, prop::MEAS_TYPE}]    = static_cast<int>(MeasurementType::ACTIVE_POWER);
        backend.ints   [{M1_H, prop::MEAS_SIDE}]    = -1;
        backend.doubles[{M1_H, prop::MEAS_VALUE}]   = 101.5;
        backend.doubles[{M1_H, prop::MEAS_STD_DEV}] = 1.0;
        backend.bools  [{M1_H, prop::MEAS_VALID}]   = true;
        backend.keyedStrings[std::make_tuple(M1_H, prop::PROPERTY, std::string("source"))] = "scada";
        backend.stringLists[{M1_H, prop::PROPERTY_NAMES}] = {"source"};

        // G_Q has no id
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
    EXPECT_THROW(Load(GEN_H, &empty).getMeasurements(), ExtensionNotFoundException);
}

TEST_F(MeasurementsTest, ListAndFilterByType) {
    Measurements meas = Generator(GEN_H, &backend).getMeasurements();
    EXPECT_EQ(meas, Measurements(EXT_H, &backend));
    std::vector<Measurement> all = meas.getMeasurements();
    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].getId(), iidm::optional<std::string>("G_P"));
    EXPECT_FALSE(all[1].getId());
    std::vector<Measurement> reactive = meas.getMeasurements(MeasurementType::REACTIVE_POWER);
    ASSERT_EQ(reactive.size(), 1u);
    EXPECT_EQ(reactive[0], Measurement(M2_H, &backend));
    EXPECT_TRUE(meas.getMeasurements(MeasurementType::VOLTAGE).empty());
}

TEST_F(MeasurementsTest, LookupById) {
    Measurements meas = Generator(GEN_H, &backend).getMeasurements();
    iidm::optional<Measurement> p = meas.getMeasurement("G_P");
    ASSERT_TRUE(p.has_value());
    EXPECT_EQ(p->getType(), MeasurementType::ACTIVE_POWER);
    EXPECT_FALSE(meas.getMeasurement("UNKNOWN").has_value());
}

TEST_F(MeasurementsTest, MeasurementValues) {
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

TEST_F(MeasurementsTest, MeasurementProperties) {
    Measurement p(M1_H, &backend);
    EXPECT_EQ(p.getPropertyNames(), std::vector<std::string>{"source"});
    EXPECT_EQ(p.getProperty("source"), iidm::optional<std::string>("scada"));
    EXPECT_FALSE(p.getProperty("unknown"));
    EXPECT_EQ(&p.putProperty("quality", "good"), &p);
    EXPECT_EQ(p.getProperty("quality"), iidm::optional<std::string>("good"));
    p.removeProperty("quality");
    EXPECT_FALSE(p.getProperty("quality"));
}

TEST_F(MeasurementsTest, RemoveAndCleanIfEmpty) {
    Measurement(M1_H, &backend).remove();
    Generator(GEN_H, &backend).getMeasurements().cleanIfEmpty();
    ASSERT_EQ(backend.invocations.size(), 2u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(static_cast<ObjectHandle>(M1_H), prop::ACT_REMOVE));
    EXPECT_EQ(backend.invocations[1], std::make_pair(static_cast<ObjectHandle>(EXT_H), prop::ACT_CLEAN_IF_EMPTY));
}

TEST_F(MeasurementsTest, NewMeasurement) {
    static constexpr ObjectHandle NEW_M = 54;
    Measurements meas = Generator(GEN_H, &backend).getMeasurements();
    MeasurementAdder adder = meas.newMeasurement();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(static_cast<ObjectHandle>(EXT_H), prop::MEASUREMENT));
    const ObjectHandle adderHandle = backend.lastAdder;
    backend.addResults[adderHandle] = NEW_M;
    Measurement m = adder.setId("L_I1").setEnsureIdUnicity(true).putProperty("source", "pmu")
                         .setType(MeasurementType::CURRENT).setSide(ThreeSides::ONE)
                         .setValue(80.0).setStandardDeviation(0.5).setValid(true).add();
    EXPECT_EQ((backend.strings[{adderHandle, prop::MEAS_ID}]), "L_I1");
    EXPECT_TRUE((backend.bools[{adderHandle, prop::ENSURE_ID_UNICITY}]));
    EXPECT_EQ((backend.keyedStrings[std::make_tuple(adderHandle, prop::PROPERTY, std::string("source"))]), "pmu");
    EXPECT_EQ((backend.ints[{adderHandle, prop::MEAS_TYPE}]), static_cast<int>(MeasurementType::CURRENT));
    EXPECT_EQ((backend.ints[{adderHandle, prop::MEAS_SIDE}]), static_cast<int>(ThreeSides::ONE));
    EXPECT_DOUBLE_EQ((backend.doubles[{adderHandle, prop::MEAS_VALUE}]), 80.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{adderHandle, prop::MEAS_STD_DEV}]), 0.5);
    EXPECT_TRUE((backend.bools[{adderHandle, prop::MEAS_VALID}]));
    EXPECT_EQ(backend.invocations.back(), std::make_pair(adderHandle, prop::ACT_ADD));
    EXPECT_EQ(m, Measurement(NEW_M, &backend));
}

TEST_F(MeasurementsTest, NewAndRemoveExtension) {
    MockBackend b;
    Load load(GEN_H, &b);
    MeasurementsAdder adder = load.newMeasurements();
    EXPECT_EQ(b.invocations.back(), std::make_pair(static_cast<ObjectHandle>(GEN_H), prop::MEASUREMENTS));
    b.addResults[b.lastAdder] = EXT_H;
    EXPECT_EQ(adder.add(), Measurements(EXT_H, &b));

    load.removeMeasurements();
    EXPECT_EQ((b.related[{GEN_H, prop::MEASUREMENTS}]), INVALID_HANDLE);
}

class DiscreteMeasurementsTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle SW_H  = 60;
    static constexpr ObjectHandle EXT_H = 63; // the extension object
    static constexpr ObjectHandle TAP_H = 61;
    static constexpr ObjectHandle POS_H = 62;

    void SetUp() override {
        backend.related[{SW_H, prop::DISCRETE_MEASUREMENTS}] = EXT_H;
        backend.children[{EXT_H, prop::DISCRETE_MEASUREMENT}] = {TAP_H, POS_H};
        backend.relatedByString[std::make_tuple(EXT_H, prop::DISCRETE_MEASUREMENT, std::string("POS"))] = POS_H;

        backend.optStrings[{TAP_H, prop::DMEAS_ID}]       = "TAP";
        backend.ints   [{TAP_H, prop::DMEAS_TYPE}]        = static_cast<int>(DiscreteMeasurementType::TAP_POSITION);
        backend.ints   [{TAP_H, prop::DMEAS_TAP_CHANGER}] = static_cast<int>(DiscreteMeasurementTapChanger::PHASE_TAP_CHANGER_2);
        backend.ints   [{TAP_H, prop::DMEAS_VALUE_TYPE}]  = static_cast<int>(DiscreteMeasurementValueType::INT);
        backend.ints   [{TAP_H, prop::DMEAS_VALUE_INT}]   = 3;

        backend.optStrings[{POS_H, prop::DMEAS_ID}]       = "POS";
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
    EXPECT_THROW(Switch(SW_H, &empty).getDiscreteMeasurements(), ExtensionNotFoundException);
}

TEST_F(DiscreteMeasurementsTest, ListFilterAndLookup) {
    DiscreteMeasurements ext = Switch(SW_H, &backend).getDiscreteMeasurements();
    EXPECT_EQ(ext, DiscreteMeasurements(EXT_H, &backend));
    EXPECT_EQ(ext.getDiscreteMeasurements().size(), 2u);
    std::vector<DiscreteMeasurement> taps = ext.getDiscreteMeasurements(DiscreteMeasurementType::TAP_POSITION);
    ASSERT_EQ(taps.size(), 1u);
    EXPECT_EQ(taps[0].getId(), iidm::optional<std::string>("TAP"));
    iidm::optional<DiscreteMeasurement> pos = ext.getDiscreteMeasurement("POS");
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

TEST_F(DiscreteMeasurementsTest, PropertiesAndRemove) {
    DiscreteMeasurement pos(POS_H, &backend);
    pos.putProperty("source", "scada");
    EXPECT_EQ(pos.getProperty("source"), iidm::optional<std::string>("scada"));
    pos.removeProperty("source");
    EXPECT_FALSE(pos.getProperty("source"));
    EXPECT_TRUE(pos.getPropertyNames().empty());

    pos.remove();
    Switch(SW_H, &backend).getDiscreteMeasurements().cleanIfEmpty();
    ASSERT_EQ(backend.invocations.size(), 2u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(static_cast<ObjectHandle>(POS_H), prop::ACT_REMOVE));
    EXPECT_EQ(backend.invocations[1], std::make_pair(static_cast<ObjectHandle>(EXT_H), prop::ACT_CLEAN_IF_EMPTY));
}

TEST_F(DiscreteMeasurementsTest, NewDiscreteMeasurement) {
    DiscreteMeasurements ext = Switch(SW_H, &backend).getDiscreteMeasurements();
    DiscreteMeasurementAdder adder = ext.newDiscreteMeasurement();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(static_cast<ObjectHandle>(EXT_H), prop::DISCRETE_MEASUREMENT));
    const ObjectHandle adderHandle = backend.lastAdder;
    adder.setId("T_TAP").setEnsureIdUnicity(false).putProperty("source", "scada")
         .setType(DiscreteMeasurementType::TAP_POSITION)
         .setTapChanger(DiscreteMeasurementTapChanger::RATIO_TAP_CHANGER)
         .setValue(4).setValid(true);
    EXPECT_EQ((backend.strings[{adderHandle, prop::DMEAS_ID}]), "T_TAP");
    EXPECT_FALSE((backend.bools[{adderHandle, prop::ENSURE_ID_UNICITY}]));
    EXPECT_EQ((backend.keyedStrings[std::make_tuple(adderHandle, prop::PROPERTY, std::string("source"))]), "scada");
    EXPECT_EQ((backend.ints[{adderHandle, prop::DMEAS_TYPE}]), static_cast<int>(DiscreteMeasurementType::TAP_POSITION));
    EXPECT_EQ((backend.ints[{adderHandle, prop::DMEAS_TAP_CHANGER}]), static_cast<int>(DiscreteMeasurementTapChanger::RATIO_TAP_CHANGER));
    EXPECT_EQ((backend.ints[{adderHandle, prop::DMEAS_VALUE_INT}]), 4);
    EXPECT_TRUE((backend.bools[{adderHandle, prop::DMEAS_VALID}]));
    // Each value type has its field; a string literal is a string
    adder.setValue("OPEN").setValue(true);
    EXPECT_EQ((backend.strings[{adderHandle, prop::DMEAS_VALUE_STRING}]), "OPEN");
    EXPECT_TRUE((backend.bools[{adderHandle, prop::DMEAS_VALUE_BOOL}]));
}

TEST_F(DiscreteMeasurementsTest, NewAndRemoveExtension) {
    MockBackend b;
    TwoWindingsTransformer twt(SW_H, &b);
    DiscreteMeasurementsAdder adder = twt.newDiscreteMeasurements();
    EXPECT_EQ(b.invocations.back(), std::make_pair(static_cast<ObjectHandle>(SW_H), prop::DISCRETE_MEASUREMENTS));
    b.addResults[b.lastAdder] = EXT_H;
    EXPECT_EQ(adder.add(), DiscreteMeasurements(EXT_H, &b));

    twt.removeDiscreteMeasurements();
    EXPECT_EQ((b.related[{SW_H, prop::DISCRETE_MEASUREMENTS}]), INVALID_HANDLE);
}
