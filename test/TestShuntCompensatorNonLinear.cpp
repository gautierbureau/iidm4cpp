#include <gtest/gtest.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

static constexpr ObjectHandle SHUNT_HANDLE = 50;
static constexpr ObjectHandle SEC1_HANDLE  = 51;
static constexpr ObjectHandle SEC2_HANDLE  = 52;

class ShuntNonLinearTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.strings[{SHUNT_HANDLE, prop::ID}]   = "SC1";
        backend.strings[{SHUNT_HANDLE, prop::NAME}]  = "Shunt 1";
        backend.ints[{SHUNT_HANDLE, prop::SHUNT_MODEL_KIND}] = 1; // NON_LINEAR
        backend.ints[{SHUNT_HANDLE, prop::SHUNT_SECTION_COUNT}]     = 2;
        backend.ints[{SHUNT_HANDLE, prop::SHUNT_MAX_SECTION_COUNT}] = 3;

        // Two non-linear sections
        backend.children[{SHUNT_HANDLE, prop::SHUNT_SECTION}] = {SEC1_HANDLE, SEC2_HANDLE};
        backend.doubles[{SEC1_HANDLE, prop::SHUNT_SECTION_B}] = 1e-4;
        backend.doubles[{SEC1_HANDLE, prop::SHUNT_SECTION_G}] = 0.0;
        backend.doubles[{SEC2_HANDLE, prop::SHUNT_SECTION_B}] = 2e-4;
        backend.doubles[{SEC2_HANDLE, prop::SHUNT_SECTION_G}] = 1e-5;
    }
};

TEST_F(ShuntNonLinearTest, HasNonLinearModel) {
    ShuntCompensator sc(SHUNT_HANDLE, &backend);
    EXPECT_TRUE(sc.hasNonLinearModel());
    EXPECT_FALSE(sc.hasLinearModel());
}

TEST_F(ShuntNonLinearTest, GetAllSections) {
    ShuntCompensator sc(SHUNT_HANDLE, &backend);
    auto model = sc.getNonLinearModel();
    EXPECT_TRUE(model.isValid());
    auto sections = model.getAllSections();
    ASSERT_EQ(sections.size(), 2u);
    EXPECT_DOUBLE_EQ(sections[0].b, 1e-4);
    EXPECT_DOUBLE_EQ(sections[0].g, 0.0);
    EXPECT_DOUBLE_EQ(sections[1].b, 2e-4);
    EXPECT_DOUBLE_EQ(sections[1].g, 1e-5);
}

TEST_F(ShuntNonLinearTest, LinearModelWhenKindIsZero) {
    MockBackend b2;
    b2.ints[{SHUNT_HANDLE, prop::SHUNT_MODEL_KIND}] = 0; // LINEAR
    ShuntCompensator sc(SHUNT_HANDLE, &b2);
    EXPECT_TRUE(sc.hasLinearModel());
    EXPECT_FALSE(sc.hasNonLinearModel());
}

TEST_F(ShuntNonLinearTest, EmptySectionsWhenNoneRegistered) {
    MockBackend b2;
    b2.ints[{SHUNT_HANDLE, prop::SHUNT_MODEL_KIND}] = 1;
    ShuntCompensator sc(SHUNT_HANDLE, &b2);
    auto sections = sc.getNonLinearModel().getAllSections();
    EXPECT_EQ(sections.size(), 0u);
}
