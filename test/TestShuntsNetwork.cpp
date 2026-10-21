#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <cmath>
#include <string>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Shunt compensators on the real backends (see RealBackends.h): the same scenario on GraalVM and
// on JNI, with the network test/data/shunts_test.xiidm (at the EQUIPMENT validation level):
// SC_LINEAR (2 of 4 sections, 3 solved), SC_NON_LINEAR (1 of 3 sections), SC_UNSET (no section
// count). Sections have no properties there: powsybl 7.4 drops those of a section adder, hence
// those of the XIIDM file.

using namespace iidm;
using iidm::test::envPath;

namespace {

ShuntCompensator shunt(BackendProvider& b, const char* id) {
    return ShuntCompensator(b.findById(prop::SHUNT_COMPENSATOR, id), &b);
}

void checkReading(BackendProvider& b) {
    ShuntCompensator linear = shunt(b, "SC_LINEAR");
    EXPECT_EQ(linear.getSectionCount(), 2);
    EXPECT_EQ(linear.findSectionCount(), iidm::optional<int>(2));
    EXPECT_EQ(linear.getSolvedSectionCount(), iidm::optional<int>(3));
    EXPECT_EQ(linear.getMaximumSectionCount(), 4);
    EXPECT_EQ(linear.getModelType(), ShuntCompensatorModelType::LINEAR);
    ShuntCompensatorModel model = linear.getModel();
    EXPECT_TRUE(model.is<ShuntCompensatorLinearModel>());
    EXPECT_FALSE(model.is<ShuntCompensatorNonLinearModel>());
    EXPECT_EQ(model, linear.getModel<ShuntCompensatorLinearModel>());
    EXPECT_EQ(model.getProperty("maker"), iidm::optional<std::string>("abb"));
    ShuntCompensatorLinearModel linearModel = model.as<ShuntCompensatorLinearModel>();
    EXPECT_DOUBLE_EQ(linearModel.getBPerSection(), 1e-3);
    EXPECT_DOUBLE_EQ(linearModel.getGPerSection(), 1e-5);
    EXPECT_THROW(linear.getModel<ShuntCompensatorNonLinearModel>(), ValidationException);
    EXPECT_DOUBLE_EQ(linear.getB(), 2e-3);
    EXPECT_DOUBLE_EQ(linear.getG(), 2e-5);
    EXPECT_DOUBLE_EQ(linear.getB(4), 4e-3);
    EXPECT_DOUBLE_EQ(linear.getG(0), 0.0);
    EXPECT_THROW(linear.getB(5), ValidationException);
    EXPECT_THROW(linear.getB(-1), ValidationException);
    EXPECT_TRUE(linear.getVoltageRegulation()->isRegulating());

    ShuntCompensator nonLinear = shunt(b, "SC_NON_LINEAR");
    EXPECT_EQ(nonLinear.getModelType(), ShuntCompensatorModelType::NON_LINEAR);
    EXPECT_EQ(nonLinear.getMaximumSectionCount(), 3);
    EXPECT_FALSE(nonLinear.findSolvedSectionCount());
    EXPECT_THROW(nonLinear.getModel<ShuntCompensatorLinearModel>(), ValidationException);
    ShuntCompensatorNonLinearModel nonLinearModel = nonLinear.getModel<ShuntCompensatorNonLinearModel>();
    EXPECT_EQ(nonLinearModel.getProperty("stages"), iidm::optional<std::string>("3"));
    std::vector<ShuntCompensatorNonLinearModel::Section> sections = nonLinearModel.getAllSections();
    ASSERT_EQ(sections.size(), 3u);
    EXPECT_DOUBLE_EQ(sections[0].getB(), 1e-3);
    EXPECT_FALSE(sections[0].hasProperty());
    EXPECT_DOUBLE_EQ(sections[1].getB(), 2.5e-3);
    EXPECT_DOUBLE_EQ(sections[1].getG(), 1e-5);
    EXPECT_DOUBLE_EQ(nonLinear.getB(), 1e-3); // that of the first section
    EXPECT_DOUBLE_EQ(nonLinear.getB(3), 4e-3);
    EXPECT_DOUBLE_EQ(nonLinear.getG(2), 1e-5);
    EXPECT_DOUBLE_EQ(nonLinear.getB(0), 0.0);
    EXPECT_THROW(nonLinear.getG(4), ValidationException);

    ShuntCompensator unset = shunt(b, "SC_UNSET");
    EXPECT_FALSE(unset.findSectionCount());
    EXPECT_THROW(unset.getSectionCount(), ValidationException);
    EXPECT_THROW(unset.getB(), IidmException); // B at an undefined section count
    EXPECT_DOUBLE_EQ(unset.getB(1), 2e-3);
    EXPECT_TRUE(std::isnan(unset.getModel<ShuntCompensatorLinearModel>().getGPerSection()));
    EXPECT_DOUBLE_EQ(unset.getG(1), 0.0); // no G per section: none
}

void checkWriting(BackendProvider& b) {
    ShuntCompensator linear = shunt(b, "SC_LINEAR");
    ShuntCompensatorLinearModel model = linear.getModel<ShuntCompensatorLinearModel>();
    model.setBPerSection(2e-3).setGPerSection(0).setMaximumSectionCount(5);
    EXPECT_DOUBLE_EQ(linear.getB(), 4e-3);
    EXPECT_DOUBLE_EQ(linear.getG(), 0.0);
    EXPECT_EQ(linear.getMaximumSectionCount(), 5);
    linear.setSectionCount(5);
    EXPECT_EQ(linear.getSectionCount(), 5);
    EXPECT_THROW(linear.setSectionCount(6), ValidationException);
    EXPECT_THROW(model.setMaximumSectionCount(4), ValidationException); // below the section count
    model.setProperty("maker", "siemens");
    EXPECT_EQ(linear.getModel().getProperty("maker"), iidm::optional<std::string>("siemens"));
    EXPECT_TRUE(model.removeProperty("maker"));
    EXPECT_FALSE(linear.getModel().hasProperty());

    // Solved values
    linear.setSolvedSectionCount(1).setSectionCountToSolvedSectionCount();
    EXPECT_EQ(linear.getSectionCount(), 1);
    linear.setSolvedSectionCount(4);
    linear.applySolvedValues();
    EXPECT_EQ(linear.getSectionCount(), 4);
    EXPECT_THROW(linear.setSolvedSectionCount(6), ValidationException);
    linear.unsetSolvedSectionCount();
    EXPECT_FALSE(linear.getSolvedSectionCount());
    linear.setSolvedSectionCount(2);
    linear.getTerminal().setQ(-30.0);
    linear.unsetSolvedValues();
    EXPECT_FALSE(linear.getSolvedSectionCount());
    EXPECT_TRUE(std::isnan(linear.getTerminal().getQ()));
    EXPECT_EQ(linear.getSectionCount(), 4);

    // The network accepts an undefined section count (EQUIPMENT validation level)
    linear.unsetSectionCount();
    EXPECT_FALSE(linear.findSectionCount());
    linear.setSectionCount(3);
    EXPECT_EQ(linear.findSectionCount(), iidm::optional<int>(3));

    ShuntCompensator nonLinear = shunt(b, "SC_NON_LINEAR");
    ShuntCompensatorNonLinearModel::Section first
        = nonLinear.getModel<ShuntCompensatorNonLinearModel>().getAllSections()[0];
    first.setB(1.5e-3).setG(1e-6).setProperty("checked", "yes").setProperty("stage", "first");
    EXPECT_DOUBLE_EQ(nonLinear.getB(), 1.5e-3);
    EXPECT_DOUBLE_EQ(nonLinear.getG(), 1e-6);
    ShuntCompensatorNonLinearModel::Section again = nonLinear.getModel<ShuntCompensatorNonLinearModel>().getAllSections()[0];
    EXPECT_EQ(again, first);
    EXPECT_EQ(again.getPropertyNames(), (std::vector<std::string>{"checked", "stage"}));
    EXPECT_TRUE(again.removeProperty("checked"));
    EXPECT_EQ(first.getPropertyNames(), (std::vector<std::string>{"stage"}));
    nonLinear.setSectionCount(3);
    EXPECT_DOUBLE_EQ(nonLinear.getB(), 4e-3);
}

// Reading the models and their sections leaves no Java reference behind
void checkHandlesAreReleased(BackendProvider& b) {
    ShuntCompensator nonLinear = shunt(b, "SC_NON_LINEAR");
    ShuntCompensator linear = shunt(b, "SC_LINEAR");
    const std::size_t baseline = b.liveHandleCount();
    double sum = 0;
    for (int i = 0; i < 300; ++i) {
        for (const auto& section : nonLinear.getModel<ShuntCompensatorNonLinearModel>().getAllSections()) sum += section.getB();
        sum += linear.getModel<ShuntCompensatorLinearModel>().getBPerSection() + linear.getB(1);
        sum += linear.getModel().is<ShuntCompensatorLinearModel>() ? 1 : 0;
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    EXPECT_NE(sum, 0.0);
}

void checkShunts(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/shunts_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/shunts_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    checkReading(b);
    checkHandlesAreReleased(b);
    checkWriting(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(ShuntsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkShunts(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(ShuntsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("shunts_test");
    checkShunts(backend);
}
#endif
