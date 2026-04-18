#include <gtest/gtest.h>
#include <iidm/BusbarSection.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class BusbarSectionTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle BBS_HANDLE      = 60;
    static constexpr ObjectHandle TERMINAL_HANDLE = 61;

    void SetUp() override {
        backend.strings[{BBS_HANDLE, prop::ID}]    = "BBS1";
        backend.strings[{BBS_HANDLE, prop::NAME}]   = "Busbar 1";
        backend.doubles[{BBS_HANDLE, prop::BBS_V}]     = 400.0;
        backend.doubles[{BBS_HANDLE, prop::BBS_ANGLE}] = -2.5;
        backend.related[{BBS_HANDLE, prop::REL_TERMINAL}] = TERMINAL_HANDLE;
        backend.doubles[{TERMINAL_HANDLE, prop::TERMINAL_P}] = 10.0;
    }
};

TEST_F(BusbarSectionTest, GetId) {
    BusbarSection bbs(BBS_HANDLE, &backend);
    EXPECT_EQ(bbs.getId(), "BBS1");
}

TEST_F(BusbarSectionTest, GetName) {
    BusbarSection bbs(BBS_HANDLE, &backend);
    EXPECT_EQ(bbs.getName(), "Busbar 1");
}

TEST_F(BusbarSectionTest, GetV) {
    BusbarSection bbs(BBS_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(bbs.getV(), 400.0);
}

TEST_F(BusbarSectionTest, GetAngle) {
    BusbarSection bbs(BBS_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(bbs.getAngle(), -2.5);
}

TEST_F(BusbarSectionTest, GetTerminal) {
    BusbarSection bbs(BBS_HANDLE, &backend);
    Terminal t = bbs.getTerminal();
    EXPECT_TRUE(t.isValid());
    EXPECT_DOUBLE_EQ(t.getP(), 10.0);
}

TEST_F(BusbarSectionTest, IsValid) {
    BusbarSection valid(BBS_HANDLE, &backend);
    BusbarSection invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(BusbarSectionTest, Equality) {
    BusbarSection a(BBS_HANDLE, &backend);
    BusbarSection b(BBS_HANDLE, &backend);
    BusbarSection c(99, &backend);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}
