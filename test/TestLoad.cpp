#include <gtest/gtest.h>
#include <iidm/Load.h>
#include <iidm/LoadAdder.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class LoadTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle LOAD_HANDLE = 10;

    void SetUp() override {
        backend.strings[{LOAD_HANDLE, prop::ID}]   = "LOAD1";
        backend.strings[{LOAD_HANDLE, prop::NAME}]  = "Load 1";
        backend.doubles[{LOAD_HANDLE, prop::LOAD_P0}] = 100.0;
        backend.doubles[{LOAD_HANDLE, prop::LOAD_Q0}] = 20.0;
        backend.ints   [{LOAD_HANDLE, prop::LOAD_TYPE}] = 0; // UNDEFINED
    }
};

TEST_F(LoadTest, Models) {
    static constexpr ObjectHandle MODEL_H = 20;
    Load load(LOAD_HANDLE, &backend);
    EXPECT_FALSE(load.getModel());
    backend.related[{LOAD_HANDLE, prop::LOAD_MODEL}] = MODEL_H;
    backend.ints [{MODEL_H, prop::LOAD_MODEL_TYPE}] = static_cast<int>(LoadModelType::ZIP);
    backend.bools[{MODEL_H, prop::ZIP_LOAD_MODEL}] = true;
    backend.doubles[{MODEL_H, prop::ZIP_C1P}] = 0.3;
    LoadModel model = *load.getModel();
    EXPECT_EQ(model.getType(), LoadModelType::ZIP);
    ASSERT_TRUE(model.is<ZipLoadModel>());
    EXPECT_FALSE(model.is<ExponentialLoadModel>());
    ZipLoadModel zip = model.as<ZipLoadModel>();
    EXPECT_DOUBLE_EQ(zip.getC1p(), 0.3);
    EXPECT_EQ(&zip.setC0p(0.1).setC1p(0.2).setC2p(0.7).setC0q(0.3).setC1q(0.3).setC2q(0.4), &zip);
    EXPECT_DOUBLE_EQ((backend.doubles[{MODEL_H, prop::ZIP_C0P}]), 0.1);
    EXPECT_DOUBLE_EQ((backend.doubles[{MODEL_H, prop::ZIP_C2P}]), 0.7);
    EXPECT_DOUBLE_EQ((backend.doubles[{MODEL_H, prop::ZIP_C0Q}]), 0.3);
    EXPECT_DOUBLE_EQ((backend.doubles[{MODEL_H, prop::ZIP_C2Q}]), 0.4);
    ExponentialLoadModel exponential(MODEL_H, &backend);
    exponential.setNp(1.5).setNq(2.5);
    EXPECT_DOUBLE_EQ(exponential.getNp(), 1.5);
    EXPECT_DOUBLE_EQ(exponential.getNq(), 2.5);
    model.setProperty("source", "study");
    EXPECT_EQ(zip.getProperty("source"), iidm::optional<std::string>("study"));
}

TEST_F(LoadTest, SolvedValues) {
    Load load(LOAD_HANDLE, &backend);
    load.applySolvedValues();
    load.setP0ToP();
    load.setQ0ToQ();
    ASSERT_EQ(backend.invocations.size(), 3u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(LOAD_HANDLE), prop::ACT_APPLY_SOLVED_VALUES));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(LOAD_HANDLE), prop::ACT_SET_TARGET_P_TO_P));
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(LOAD_HANDLE), prop::ACT_SET_TARGET_Q_TO_Q));
}

// The model of a new load: nested in the load adder, add() returns to it
TEST_F(LoadTest, ModelAdders) {
    LoadAdder adder(backend.invoke(LOAD_HANDLE, prop::LOAD), &backend);
    const ObjectHandle loadAdder = backend.lastAdder;
    adder.setP0(1).newZipModel().setC0p(1).setC1p(0).setC2p(0).setC0q(1).setC1q(0).setC2q(0)
         .setProperty("k", "v").add().setQ0(2);
    const ObjectHandle zipAdder = loadAdder + 1;
    EXPECT_EQ(backend.invocations[1], std::make_pair(loadAdder, prop::ZIP_LOAD_MODEL));
    EXPECT_DOUBLE_EQ((backend.doubles[{zipAdder, prop::ZIP_C0P}]), 1.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{zipAdder, prop::ZIP_C0Q}]), 1.0);
    EXPECT_EQ((backend.keyedStrings[std::make_tuple(zipAdder, prop::PROPERTY, std::string("k"))]), "v");
    EXPECT_EQ(backend.invocations[2], std::make_pair(zipAdder, prop::ACT_ADD));
    EXPECT_DOUBLE_EQ((backend.doubles[{loadAdder, prop::LOAD_Q0}]), 2.0);

    adder.newExponentialModel().setNp(1.2).setNq(2.2).add();
    const ObjectHandle expAdder = backend.lastAdder;
    EXPECT_EQ(backend.invocations[3], std::make_pair(loadAdder, prop::EXPONENTIAL_LOAD_MODEL));
    EXPECT_DOUBLE_EQ((backend.doubles[{expAdder, prop::EXP_NQ}]), 2.2);
    EXPECT_EQ(backend.invocations[4], std::make_pair(expAdder, prop::ACT_ADD));
}
