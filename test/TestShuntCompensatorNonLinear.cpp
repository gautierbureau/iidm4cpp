#include <gtest/gtest.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <iidm/Terminal.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

static constexpr ObjectHandle SHUNT_HANDLE = 50;
static constexpr ObjectHandle SEC1_HANDLE  = 51;
static constexpr ObjectHandle SEC2_HANDLE  = 52;
static constexpr ObjectHandle MODEL_HANDLE = 53;

class ShuntNonLinearTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.strings[{SHUNT_HANDLE, prop::ID}]   = "SC1";
        backend.strings[{SHUNT_HANDLE, prop::NAME}]  = "Shunt 1";
        backend.ints[{SHUNT_HANDLE, prop::SHUNT_MODEL_TYPE}] = static_cast<int>(ShuntCompensatorModelType::NON_LINEAR);
        backend.ints[{SHUNT_HANDLE, prop::SHUNT_SECTION_COUNT}]     = 2;
        backend.ints[{SHUNT_HANDLE, prop::SHUNT_MAX_SECTION_COUNT}] = 3;
        backend.related[{SHUNT_HANDLE, prop::SHUNT_COMPENSATOR_NON_LINEAR_MODEL}] = MODEL_HANDLE;
        backend.bools[{MODEL_HANDLE, prop::SHUNT_COMPENSATOR_NON_LINEAR_MODEL}] = true;

        // Two non-linear sections
        backend.children[{MODEL_HANDLE, prop::SHUNT_SECTION}] = {SEC1_HANDLE, SEC2_HANDLE};
        backend.doubles[{SEC1_HANDLE, prop::SHUNT_SECTION_B}] = 1e-4;
        backend.doubles[{SEC1_HANDLE, prop::SHUNT_SECTION_G}] = 0.0;
        backend.doubles[{SEC2_HANDLE, prop::SHUNT_SECTION_B}] = 2e-4;
        backend.doubles[{SEC2_HANDLE, prop::SHUNT_SECTION_G}] = 1e-5;
    }
};

TEST_F(ShuntNonLinearTest, ModelType) {
    ShuntCompensator sc(SHUNT_HANDLE, &backend);
    EXPECT_EQ(sc.getModelType(), ShuntCompensatorModelType::NON_LINEAR);
    ShuntCompensatorNonLinearModel model = sc.getModel<ShuntCompensatorNonLinearModel>();
    EXPECT_TRUE(model.isValid());
    EXPECT_TRUE(model.is<ShuntCompensatorNonLinearModel>());
    EXPECT_FALSE(model.is<ShuntCompensatorLinearModel>());
}

TEST_F(ShuntNonLinearTest, GetAllSections) {
    ShuntCompensator sc(SHUNT_HANDLE, &backend);
    std::vector<ShuntCompensatorNonLinearModel::Section> sections
        = sc.getModel<ShuntCompensatorNonLinearModel>().getAllSections();
    ASSERT_EQ(sections.size(), 2u);
    EXPECT_DOUBLE_EQ(sections[0].getB(), 1e-4);
    EXPECT_DOUBLE_EQ(sections[0].getG(), 0.0);
    EXPECT_DOUBLE_EQ(sections[1].getB(), 2e-4);
    EXPECT_DOUBLE_EQ(sections[1].getG(), 1e-5);
}

TEST_F(ShuntNonLinearTest, SetSections) {
    ShuntCompensatorNonLinearModel::Section section(SEC2_HANDLE, &backend);
    EXPECT_EQ(&section.setB(3e-4).setG(2e-5), &section);
    EXPECT_DOUBLE_EQ((backend.doubles[{SEC2_HANDLE, prop::SHUNT_SECTION_B}]), 3e-4);
    EXPECT_DOUBLE_EQ((backend.doubles[{SEC2_HANDLE, prop::SHUNT_SECTION_G}]), 2e-5);
    section.setProperty("stage", "second");
    EXPECT_EQ(section.getProperty("stage"), iidm::optional<std::string>("second"));
}

TEST_F(ShuntNonLinearTest, EmptySectionsWhenNoneRegistered) {
    MockBackend b2;
    ShuntCompensatorNonLinearModel model(MODEL_HANDLE, &b2);
    EXPECT_TRUE(model.getAllSections().empty());
}

// ── Regulating terminal (the voltage regulation: see TestVoltageRegulationHolder) ──

static constexpr ObjectHandle TERM_HANDLE2 = 60;

TEST_F(ShuntNonLinearTest, GetRegulatingTerminal) {
    backend.related[{SHUNT_HANDLE, prop::REL_REGULATING_TERMINAL}] = TERM_HANDLE2;
    backend.doubles[{TERM_HANDLE2, prop::TERMINAL_P}] = 10.0;
    ShuntCompensator sc(SHUNT_HANDLE, &backend);
    Terminal t = sc.getRegulatingTerminal();
    EXPECT_TRUE(t.isValid());
    EXPECT_DOUBLE_EQ(t.getP(), 10.0);
}

TEST_F(ShuntNonLinearTest, GetRegulatingTerminalInvalidWhenMissing) {
    ShuntCompensator sc(SHUNT_HANDLE, &backend);
    Terminal t = sc.getRegulatingTerminal();
    EXPECT_FALSE(t.isValid());
}

// ── ShuntCompensator::getB ────────────────────────────────────────────────────

TEST_F(ShuntNonLinearTest, GetB) {
    backend.doubles[{SHUNT_HANDLE, prop::SHUNT_B}] = 3e-4;
    ShuntCompensator sc(SHUNT_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(sc.getB(), 3e-4);
}
