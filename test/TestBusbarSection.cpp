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

TEST_F(BusbarSectionTest, GetTerminal) {
    BusbarSection bbs(BBS_HANDLE, &backend);
    Terminal t = bbs.getTerminal();
    EXPECT_TRUE(t.isValid());
    EXPECT_DOUBLE_EQ(t.getP(), 10.0);
}

