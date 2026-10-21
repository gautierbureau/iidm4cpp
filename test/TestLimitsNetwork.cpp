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

// Limits on the real backends (see RealBackends.h): the same scenario on GraalVM and on JNI, with
// the network test/data/transformers_test.xiidm (T2: limits of each type on side 1, two selected
// groups on side 2, low limits; T3 legs and the boundary line DL).

using namespace iidm;
using iidm::test::envPath;

namespace {

using Strings = std::vector<std::string>;

void checkReading(BackendProvider& b) {
    TwoWindingsTransformer t2(b.findById(prop::TWO_WINDINGS_TRANSFORMER, "T2"), &b);

    // Side 1: one group, selected, with current and active power limits
    EXPECT_EQ(t2.getSelectedOperationalLimitsGroupId1(), iidm::optional<std::string>("DEFAULT"));
    OperationalLimitsGroup group = *t2.getSelectedOperationalLimitsGroup1();
    EXPECT_EQ(*group.getProperty("season"), "all");
    EXPECT_FALSE(group.isEmpty());
    CurrentLimits current = *t2.getCurrentLimits1();
    EXPECT_EQ(current, *group.getCurrentLimits());
    EXPECT_EQ(current.getLimitType(), LimitType::CURRENT);
    EXPECT_EQ(current.getDetectionKind(), DetectionKind::HIGH);
    EXPECT_DOUBLE_EQ(current.getPermanentLimit(), 800.0);
    EXPECT_EQ(current.getPermanentLimitName(), "PATL");
    EXPECT_EQ(*current.getProperty("checked"), "yes");
    std::vector<TemporaryLimit> temporary = current.getTemporaryLimits();
    ASSERT_EQ(temporary.size(), 2u);
    EXPECT_EQ(temporary[0].getAcceptableDuration(), 600); // by decreasing acceptable duration
    EXPECT_EQ(temporary[0].getName(), "TL10");
    EXPECT_EQ(*temporary[0].getProperty("source"), "rte");
    EXPECT_EQ(temporary[1].getName(), "TL1");
    EXPECT_TRUE(temporary[1].isFictitious());
    EXPECT_EQ(current.getTemporaryLimit(60)->getName(), "TL1");
    EXPECT_FALSE(current.getTemporaryLimit(61));
    EXPECT_DOUBLE_EQ(current.getTemporaryLimitValue(600), 900.0);
    EXPECT_TRUE(std::isnan(current.getTemporaryLimitValue(61)));
    EXPECT_DOUBLE_EQ(t2.getActivePowerLimits1()->getPermanentLimit(), 300.0);
    EXPECT_FALSE(t2.getApparentPowerLimits1());
    EXPECT_EQ(*t2.getLimits(LimitType::ACTIVE_POWER, TwoSides::ONE), *t2.getActivePowerLimits1());
    EXPECT_TRUE(t2.getLimits(LimitType::ACTIVE_POWER, TwoSides::ONE)->is<ActivePowerLimits>());
    EXPECT_FALSE(t2.getLimits(LimitType::ACTIVE_POWER, TwoSides::ONE)->is<CurrentLimits>());

    // Side 2: three groups, two of them selected
    EXPECT_EQ(t2.getOperationalLimitsGroups2().size(), 3u);
    EXPECT_EQ(t2.getAllSelectedOperationalLimitsGroupIds(TwoSides::TWO), (Strings{"SUMMER", "WINTER"}));
    EXPECT_EQ(t2.getAllSelectedOperationalLimitsGroupIdsOrdered(TwoSides::TWO), (Strings{"WINTER", "SUMMER"}));
    EXPECT_EQ(t2.getAllSelectedOperationalLimitsGroups(TwoSides::TWO).size(), 2u);
    ASSERT_EQ(t2.getAllSelectedCurrentLimits(TwoSides::TWO).size(), 1u);
    EXPECT_DOUBLE_EQ(t2.getAllSelectedCurrentLimits(TwoSides::TWO)[0].getPermanentLimit(), 700.0);
    EXPECT_DOUBLE_EQ(t2.getAllSelectedLimits(LimitType::APPARENT_POWER, TwoSides::TWO)[0].getPermanentLimit(), 350.0);
    CurrentLimits low = *t2.getOperationalLimitsGroup2("LOW")->getCurrentLimits();
    EXPECT_EQ(low.getDetectionKind(), DetectionKind::LOW);
    EXPECT_THROW(low.getPermanentLimit(), ValidationException); // low limits have none
    EXPECT_DOUBLE_EQ(low.getTemporaryLimitValue(600), 10.0);
    EXPECT_FALSE(t2.getOperationalLimitsGroup2("UNKNOWN"));

    EXPECT_EQ(t2.getSide(t2.getTerminal2()), TwoSides::TWO);
    EXPECT_THROW(t2.getSide(Load(b.findById(prop::LOAD, "L2"), &b).getTerminal()), ValidationException);

    // Legs and the boundary line hold one set of limits
    ThreeWindingsTransformer t3(b.findById(prop::THREE_WINDINGS_TRANSFORMER, "T3"), &b);
    EXPECT_DOUBLE_EQ(t3.getLeg1().getLimits(LimitType::CURRENT)->getPermanentLimit(), 400.0);
    EXPECT_EQ(t3.getLeg1().getAllSelectedLimits(LimitType::CURRENT).size(), 1u);
    EXPECT_DOUBLE_EQ(t3.getLeg1().getCurrentLimitsFromId("SUMMER")->getPermanentLimit(), 350.0);
    DanglingLine dl(b.findById(prop::DANGLING_LINE, "DL"), &b);
    EXPECT_EQ(dl.getAllSelectedOperationalLimitsGroupIds(), (Strings{"DEFAULT"}));
}

void checkWriting(BackendProvider& b) {
    TwoWindingsTransformer t2(b.findById(prop::TWO_WINDINGS_TRANSFORMER, "T2"), &b);
    CurrentLimits current = *t2.getCurrentLimits1();
    current.setPermanentLimit(850.0).setPermanentLimitName("IST");
    EXPECT_DOUBLE_EQ(t2.getCurrentLimits1()->getPermanentLimit(), 850.0);
    EXPECT_EQ(t2.getCurrentLimits1()->getPermanentLimitName(), "IST");
    current.setTemporaryLimitValue(60, 1100.0);
    EXPECT_DOUBLE_EQ(current.getTemporaryLimit(60)->getValue(), 1100.0);
    EXPECT_THROW(current.setTemporaryLimitValue(61, 1100.0), ValidationException);

    // A new group, with limits, then the only one selected
    OperationalLimitsGroup created = t2.newOperationalLimitsGroup2("NEW");
    EXPECT_TRUE(created.isEmpty());
    CurrentLimits added = created.newCurrentLimits().setPermanentLimit(500.0).setProperty("origin", "c++")
        .beginTemporaryLimit().setName("T5").setAcceptableDuration(300).setValue(550.0).addProperty("k", "v").endTemporaryLimit()
        .beginTemporaryLimit().setName("T5").ensureNameUnicity().setAcceptableDuration(60).setValue(600.0).endTemporaryLimit()
        .add();
    EXPECT_EQ(added, *created.getCurrentLimits());
    EXPECT_EQ(*added.getProperty("origin"), "c++");
    EXPECT_EQ(*added.getTemporaryLimit(300)->getProperty("k"), "v");
    EXPECT_NE(added.getTemporaryLimit(60)->getName(), "T5");
    created.newApparentPowerLimits().setPermanentLimit(400.0).add();
    t2.setSelectedOperationalLimitsGroup2("NEW");
    EXPECT_EQ(t2.getAllSelectedOperationalLimitsGroupIds(TwoSides::TWO), (Strings{"NEW"}));
    EXPECT_DOUBLE_EQ(t2.getCurrentLimits2()->getPermanentLimit(), 500.0);
    EXPECT_DOUBLE_EQ(t2.getApparentPowerLimits(TwoSides::TWO)->getPermanentLimit(), 400.0);
    EXPECT_THROW(t2.setSelectedOperationalLimitsGroup1("UNKNOWN"), ValidationException);

    t2.addSelectedOperationalLimitsGroups(TwoSides::TWO, {"WINTER", "SUMMER"});
    t2.deselectOperationalLimitsGroups(TwoSides::TWO, {"NEW", "SUMMER"});
    EXPECT_EQ(t2.getAllSelectedOperationalLimitsGroupIds(TwoSides::TWO), (Strings{"WINTER"}));
    t2.addSelectedOperationalLimitsGroupByPredicate(TwoSides::TWO, [](const std::string& id) { return id.find('E') != std::string::npos; });
    EXPECT_EQ(t2.getAllSelectedOperationalLimitsGroupIds(TwoSides::TWO), (Strings{"NEW", "SUMMER", "WINTER"}));
    t2.deselectOperationalLimitsGroups(TwoSides::TWO, {"NEW", "SUMMER"});
    t2.cancelSelectedOperationalLimitsGroup2();
    EXPECT_FALSE(t2.getSelectedOperationalLimitsGroup2());
    EXPECT_TRUE(t2.getAllSelectedOperationalLimitsGroupIds(TwoSides::TWO).empty());
    EXPECT_EQ(t2.getOrCreateSelectedOperationalLimitsGroup2("SUMMER").getId(), "SUMMER");
    EXPECT_EQ(t2.getSelectedOperationalLimitsGroupId2(), iidm::optional<std::string>("SUMMER"));

    created.removeApparentPowerLimits();
    EXPECT_FALSE(created.getApparentPowerLimits());
    t2.removeOperationalLimitsGroup2("NEW");
    EXPECT_FALSE(t2.getOperationalLimitsGroup2("NEW"));
    t2.getActivePowerLimits1()->remove();
    EXPECT_FALSE(t2.getActivePowerLimits1());

    DanglingLine dl(b.findById(prop::DANGLING_LINE, "DL"), &b);
    dl.newOperationalLimitsGroup("EXTRA").newActivePowerLimits().setPermanentLimit(10.0).add();
    dl.setSelectedOperationalLimitsGroup("EXTRA");
    EXPECT_DOUBLE_EQ(dl.getActivePowerLimits()->getPermanentLimit(), 10.0);
    EXPECT_FALSE(dl.getCurrentLimits());
    EXPECT_DOUBLE_EQ(dl.getCurrentLimitsFromId("DEFAULT")->getPermanentLimit(), 1000.0);
}

// Reading limits through the branch sides leaves no Java reference behind
void checkHandlesAreReleased(BackendProvider& b) {
    TwoWindingsTransformer t2(b.findById(prop::TWO_WINDINGS_TRANSFORMER, "T2"), &b);
    const std::size_t baseline = b.liveHandleCount();
    double sum = 0;
    for (int i = 0; i < 300; ++i) {
        for (const TemporaryLimit& limit : t2.getCurrentLimits1()->getTemporaryLimits()) sum += limit.getValue();
        sum += static_cast<double>(t2.getAllSelectedOperationalLimitsGroups(TwoSides::TWO).size());
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    EXPECT_NE(sum, 0.0);
}

void checkLimits(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/transformers_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/transformers_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    checkReading(b);
    checkHandlesAreReleased(b);
    checkWriting(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(LimitsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkLimits(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(LimitsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("limits_test");
    checkLimits(backend);
}
#endif
