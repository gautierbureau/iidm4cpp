#include <gtest/gtest.h>
#include <iidm/DanglingLine.h>
#include <iidm/CurrentLimits.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class DanglingLineTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle DL_HANDLE = 10;
    static constexpr ObjectHandle CL_HANDLE = 11;

    void SetUp() override {
        backend.strings[{DL_HANDLE, prop::ID}]    = "DL1";
        backend.strings[{DL_HANDLE, prop::NAME}]   = "Dangling Line 1";
        backend.doubles[{DL_HANDLE, prop::DL_P0}]  = 50.0;
        backend.doubles[{DL_HANDLE, prop::DL_Q0}]  = 10.0;
        backend.doubles[{DL_HANDLE, prop::DL_R}]   = 1.0;
    }
};

TEST_F(DanglingLineTest, GetId) {
    DanglingLine dl(DL_HANDLE, &backend);
    EXPECT_EQ(dl.getId(), "DL1");
}

// ── getCurrentLimits ──────────────────────────────────────────────────────────

TEST_F(DanglingLineTest, GetCurrentLimitsAbsent) {
    DanglingLine dl(DL_HANDLE, &backend);
    EXPECT_FALSE(dl.getCurrentLimits().has_value());
}

TEST_F(DanglingLineTest, GetCurrentLimitsPresent) {
    backend.related[{DL_HANDLE, prop::REL_CURRENT_LIMITS1}] = CL_HANDLE;
    backend.doubles[{CL_HANDLE, prop::CL_PERMANENT_LIMIT}]  = 300.0;
    DanglingLine dl(DL_HANDLE, &backend);
    auto cl = dl.getCurrentLimits();
    ASSERT_TRUE(cl.has_value());
    EXPECT_DOUBLE_EQ(cl->getPermanentLimit(), 300.0);
}
