#include <gtest/gtest.h>
#include <iidm/ObservabilityArea.h>
#include <iidm/PropertyCodes.h>
#include <iidm/VoltageLevel.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class ObservabilityAreaTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle VL_H     = 70;
    static constexpr ObjectHandle AREA1_H  = 71;
    static constexpr ObjectHandle AREA2_H  = 72;
    static constexpr ObjectHandle TERM_H   = 73;
    static constexpr ObjectHandle BUS_H    = 74;
    static constexpr ObjectHandle EXT_H    = 75; // the extension object

    void SetUp() override {
        backend.related[{VL_H, prop::OBSERVABILITY_AREA}] = EXT_H;
        backend.bools[{EXT_H, prop::EXT_OBS_AREA_CONSISTENT}] = true;
        backend.children[{EXT_H, prop::AREA_CHARACTERISTICS}] = {AREA1_H, AREA2_H};
        backend.relatedByIndex[std::make_tuple(EXT_H, prop::REL_OBS_AREA_BY_NODE, 4)] = AREA1_H;
        backend.relatedByString[std::make_tuple(EXT_H, prop::REL_OBS_AREA_BY_BUS, std::string("VL_0"))] = AREA1_H;
        backend.relatedByString[std::make_tuple(EXT_H, prop::REL_OBS_AREA_BY_BBV_BUS, std::string("B2"))] = AREA2_H;

        backend.ints[{AREA1_H, prop::OBS_AREA_NUMBER}] = 7;
        backend.ints[{AREA1_H, prop::OBS_AREA_STATUS}] = static_cast<int>(ObservabilityStatus::NON_OBSERVABLE);
        backend.intLists[{AREA1_H, prop::OBS_AREA_NODES}] = {0, 4, 5};
        backend.children[{AREA1_H, prop::OBS_AREA_TERMINALS}] = {TERM_H};

        backend.ints[{AREA2_H, prop::OBS_AREA_NUMBER}] = 2;
        backend.ints[{AREA2_H, prop::OBS_AREA_STATUS}] = static_cast<int>(ObservabilityStatus::BORDER);
        backend.children[{AREA2_H, prop::OBS_AREA_BUSES}] = {BUS_H};
    }
};

TEST_F(ObservabilityAreaTest, Areas) {
    auto ext = VoltageLevel(VL_H, &backend).getExtension<ObservabilityArea>();
    EXPECT_TRUE(ext.isConsistentWithTopology());
    auto areas = ext.getObservabilityAreas();
    ASSERT_EQ(areas.size(), 2u);
    EXPECT_EQ(areas[0].getAreaNumber(), 7);
    EXPECT_EQ(areas[0].getStatus(), ObservabilityStatus::NON_OBSERVABLE);
    EXPECT_EQ(areas[0].getNodes(), (std::vector<int>{0, 4, 5}));
    ASSERT_EQ(areas[0].getTerminals().size(), 1u);
    EXPECT_TRUE(areas[0].getBuses().empty());
    EXPECT_EQ(areas[1].getStatus(), ObservabilityStatus::BORDER);
    ASSERT_EQ(areas[1].getBuses().size(), 1u);
    EXPECT_TRUE(areas[1].getNodes().empty());
}

TEST_F(ObservabilityAreaTest, Lookups) {
    auto ext = VoltageLevel(VL_H, &backend).getExtension<ObservabilityArea>();
    auto byNode = ext.getObservabilityAreaByNode(4);
    ASSERT_TRUE(byNode);
    EXPECT_EQ(byNode->getAreaNumber(), 7);
    EXPECT_FALSE(ext.getObservabilityAreaByNode(9));
    auto byBus = ext.getObservabilityAreaByBus("VL_0");
    ASSERT_TRUE(byBus);
    EXPECT_EQ(byBus->getAreaNumber(), 7);
    auto byBbvBus = ext.getObservabilityAreaByBusBreakerBus("B2");
    ASSERT_TRUE(byBbvBus);
    EXPECT_EQ(byBbvBus->getAreaNumber(), 2);
    EXPECT_FALSE(ext.getObservabilityAreaByBusBreakerBus("UNKNOWN"));
}
