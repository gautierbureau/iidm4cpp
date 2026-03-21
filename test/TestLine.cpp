#include <gtest/gtest.h>
#include <iidm/Line.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class LineTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle LINE_HANDLE  = 50;
    static constexpr ObjectHandle TERM1_HANDLE = 51;
    static constexpr ObjectHandle TERM2_HANDLE = 52;

    void SetUp() override {
        backend.strings[{LINE_HANDLE, prop::ID}]   = "LINE1";
        backend.strings[{LINE_HANDLE, prop::NAME}]  = "Line 1";
        backend.doubles[{LINE_HANDLE, prop::LINE_R}]  = 0.01;
        backend.doubles[{LINE_HANDLE, prop::LINE_X}]  = 0.1;
        backend.doubles[{LINE_HANDLE, prop::LINE_G1}] = 0.0;
        backend.doubles[{LINE_HANDLE, prop::LINE_B1}] = 1e-4;
        backend.doubles[{LINE_HANDLE, prop::LINE_G2}] = 0.0;
        backend.doubles[{LINE_HANDLE, prop::LINE_B2}] = 1e-4;
        backend.related[{LINE_HANDLE, prop::REL_TERMINAL1}] = TERM1_HANDLE;
        backend.related[{LINE_HANDLE, prop::REL_TERMINAL2}] = TERM2_HANDLE;
    }
};

TEST_F(LineTest, GetId) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_EQ(line.getId(), "LINE1");
}

TEST_F(LineTest, GetRX) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(line.getR(), 0.01);
    EXPECT_DOUBLE_EQ(line.getX(), 0.1);
}

TEST_F(LineTest, GetG1B1G2B2) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(line.getG1(), 0.0);
    EXPECT_DOUBLE_EQ(line.getB1(), 1e-4);
    EXPECT_DOUBLE_EQ(line.getG2(), 0.0);
    EXPECT_DOUBLE_EQ(line.getB2(), 1e-4);
}

TEST_F(LineTest, GetTerminals) {
    Line line(LINE_HANDLE, &backend);
    Terminal t1 = line.getTerminal1();
    Terminal t2 = line.getTerminal2();
    EXPECT_TRUE(t1.isValid());
    EXPECT_TRUE(t2.isValid());
    EXPECT_NE(t1, t2);
}
