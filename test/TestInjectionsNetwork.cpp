#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <cmath>
#include <string>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Injections on the real backends (see RealBackends.h): the same scenario on GraalVM and on JNI,
// with the network test/data/injections_test.xiidm: loads with a ZIP model (L_ZIP), an
// exponential model (L_EXP) and none (L_PLAIN), a generator G, a battery BAT, an LCC converter
// station LCC; solved flows.

using namespace iidm;
using iidm::test::envPath;

namespace {

Load load(BackendProvider& b, const char* id) {
    return Load(b.findById(prop::LOAD, id), &b);
}

void checkLoads(BackendProvider& b) {
    Load zip = load(b, "L_ZIP");
    ASSERT_TRUE(zip.getModel());
    LoadModel model = *zip.getModel();
    EXPECT_EQ(model.getType(), LoadModelType::ZIP);
    EXPECT_TRUE(model.is<ZipLoadModel>());
    EXPECT_EQ(model, *zip.getModel());
    EXPECT_EQ(model.getProperty("source"), iidm::optional<std::string>("study"));
    ZipLoadModel zipModel = model.as<ZipLoadModel>();
    EXPECT_DOUBLE_EQ(zipModel.getC0p(), 0.5);
    EXPECT_DOUBLE_EQ(zipModel.getC2q(), 0.2);
    zipModel.setC0p(0.4).setC1p(0.4);
    EXPECT_DOUBLE_EQ(zip.getModel()->as<ZipLoadModel>().getC1p(), 0.4);

    Load exp = load(b, "L_EXP");
    EXPECT_EQ(exp.getModel()->getType(), LoadModelType::EXPONENTIAL);
    ExponentialLoadModel expModel = exp.getModel()->as<ExponentialLoadModel>();
    EXPECT_DOUBLE_EQ(expModel.getNp(), 1.2);
    EXPECT_DOUBLE_EQ(expModel.getNq(), 2.1);
    expModel.setNq(1.9);
    EXPECT_DOUBLE_EQ(exp.getModel()->as<ExponentialLoadModel>().getNq(), 1.9);
    EXPECT_THROW(exp.getModel()->as<ZipLoadModel>(), PropertyNotFoundException);

    Load plain = load(b, "L_PLAIN");
    EXPECT_FALSE(plain.getModel());
    EXPECT_EQ(plain.getLoadType(), LoadType::AUXILIARY);
    plain.setLoadType(LoadType::FICTITIOUS);
    EXPECT_EQ(plain.getLoadType(), LoadType::FICTITIOUS);

    // P0 and Q0 from the solved flows
    zip.setP0ToP();
    EXPECT_DOUBLE_EQ(zip.getP0(), 41.0);
    EXPECT_DOUBLE_EQ(zip.getQ0(), 10.0);
    zip.setQ0ToQ();
    EXPECT_DOUBLE_EQ(zip.getQ0(), 11.0);
    exp.applySolvedValues();
    EXPECT_DOUBLE_EQ(exp.getP0(), 19.5);
    EXPECT_DOUBLE_EQ(exp.getQ0(), 4.8);
}

void checkLoadCreation(BackendProvider& b) {
    VoltageLevel vl(b.findById(prop::VOLTAGE_LEVEL, "VL"), &b);
    Load created = vl.newLoad().setId("L_NEW").setBus("B").setP0(3).setQ0(1)
        .newExponentialModel().setNp(0.8).setNq(1.6).setProperty("k", "v").add()
        .add();
    ASSERT_TRUE(created.getModel());
    EXPECT_DOUBLE_EQ(created.getModel()->as<ExponentialLoadModel>().getNq(), 1.6);
    EXPECT_EQ(created.getModel()->getProperty("k"), iidm::optional<std::string>("v"));
    Load zipCreated = vl.newLoad().setId("L_NEW_ZIP").setBus("B").setP0(3).setQ0(1)
        .newZipModel().setC0p(1).setC1p(0).setC2p(0).setC0q(1).setC1q(0).setC2q(0).add()
        .add();
    EXPECT_EQ(zipCreated.getModel()->getType(), LoadModelType::ZIP);
    // The parts must sum to 1
    EXPECT_THROW(vl.newLoad().setId("L_BAD").setBus("B").setP0(1).setQ0(1)
                     .newZipModel().setC0p(1).setC1p(1).setC2p(0).setC0q(1).setC1q(0).setC2q(0).add(),
                 ValidationException);
}

void checkOtherInjections(BackendProvider& b) {
    Generator g(b.findById(prop::GENERATOR, "G"), &b);
    g.setTargetPToP();
    EXPECT_DOUBLE_EQ(g.getTargetP(), 72.0);
    g.getTerminal().setP(-75).setQ(-15);
    g.applySolvedValues();
    EXPECT_DOUBLE_EQ(g.getTargetP(), 75.0);

    Battery battery(b.findById(prop::BATTERY, "BAT"), &b);
    battery.setMinP(-30).setMaxP(40);
    EXPECT_DOUBLE_EQ(battery.getMinP(), -30.0);
    EXPECT_DOUBLE_EQ(battery.getMaxP(), 40.0);
    EXPECT_THROW(battery.setMaxP(-40), ValidationException); // below the minimum
    battery.setTargetPToP();
    EXPECT_DOUBLE_EQ(battery.getTargetP(), 4.0);
    battery.getTerminal().setP(-6).setQ(2);
    battery.applySolvedValues();
    EXPECT_DOUBLE_EQ(battery.getTargetP(), 6.0);

    LccConverterStation lcc(b.findById(prop::LCC_CONVERTER_STATION, "LCC"), &b);
    EXPECT_NEAR(lcc.getPowerFactor(), 0.8, 1e-6); // a float in powsybl
    lcc.setPowerFactor(0.9);
    EXPECT_NEAR(lcc.getPowerFactor(), 0.9, 1e-6);
}

// Reading the load models leaves no Java reference behind
void checkHandlesAreReleased(BackendProvider& b) {
    Load zip = load(b, "L_ZIP");
    const std::size_t baseline = b.liveHandleCount();
    double sum = 0;
    for (int i = 0; i < 300; ++i) {
        sum += zip.getModel()->as<ZipLoadModel>().getC0p();
        sum += zip.getModel()->is<ExponentialLoadModel>() ? 1 : 0;
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    EXPECT_NE(sum, 0.0);
}

void checkInjections(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/injections_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/injections_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    checkHandlesAreReleased(b);
    checkLoads(b);
    checkLoadCreation(b);
    checkOtherInjections(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(InjectionsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkInjections(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(InjectionsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("injections_test");
    checkInjections(backend);
}
#endif
