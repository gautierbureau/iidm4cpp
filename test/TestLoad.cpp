#include <gtest/gtest.h>
#include <iidm/Load.h>
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

TEST_F(LoadTest, GetId) {
    Load load(LOAD_HANDLE, &backend);
    EXPECT_EQ(load.getId(), "LOAD1");
}

TEST_F(LoadTest, GetP0Q0) {
    Load load(LOAD_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(load.getP0(), 100.0);
    EXPECT_DOUBLE_EQ(load.getQ0(), 20.0);
}

TEST_F(LoadTest, SetP0Q0) {
    Load load(LOAD_HANDLE, &backend);
    load.setP0(120.0).setQ0(25.0);
    EXPECT_DOUBLE_EQ(backend.doubles[{LOAD_HANDLE, prop::LOAD_P0}], 120.0);
    EXPECT_DOUBLE_EQ(backend.doubles[{LOAD_HANDLE, prop::LOAD_Q0}], 25.0);
}

TEST_F(LoadTest, GetLoadType) {
    Load load(LOAD_HANDLE, &backend);
    EXPECT_EQ(load.getLoadType(), LoadType::UNDEFINED);
}

TEST_F(LoadTest, IsValid) {
    Load valid(LOAD_HANDLE, &backend);
    Load invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
