#include <gtest/gtest.h>
#include <iidm/Line.h>
#include <iidm/Overload.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ThreeWindingsTransformer.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

// The overload checks of branches and three windings transformers, as the backend sees them: the
// permanent limits evaluated, the temporary limits checked by a query that ACT_ADD runs.

static constexpr ObjectHandle LINE_HANDLE = 10;
static constexpr ObjectHandle T3_HANDLE = 11;
static constexpr ObjectHandle OVERLOAD_HANDLE = 20;
static constexpr ObjectHandle TL_HANDLE = 21;
static constexpr ObjectHandle LIST_HANDLE = 22;
static constexpr ObjectHandle CHECK_HANDLE = 500; // the next "adder" of the mock

static double current() { return static_cast<double>(LimitType::CURRENT); }

class OverloadsTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.nextAdder = CHECK_HANDLE;
        backend.doubles[{OVERLOAD_HANDLE, prop::OVERLOAD_PREVIOUS_LIMIT}] = 900.0;
        backend.strings[{OVERLOAD_HANDLE, prop::OVERLOAD_PREVIOUS_LIMIT_NAME}] = "TL10";
        backend.strings[{OVERLOAD_HANDLE, prop::OVERLOAD_OLG_ID}] = "DEFAULT";
        backend.doubles[{OVERLOAD_HANDLE, prop::OVERLOAD_LIMIT_SCALING}] = 1.0;
        backend.related[{OVERLOAD_HANDLE, prop::TEMPORARY_LIMIT}] = TL_HANDLE;
        backend.strings[{TL_HANDLE, prop::TL_NAME}] = "TL1";
    }
};

TEST_F(OverloadsTest, BranchPermanentLimits) {
    backend.bools[{LINE_HANDLE, prop::OVERLOADED}] = true;
    backend.evaluations[std::make_tuple(ObjectHandle(LINE_HANDLE), prop::OVERLOADED, std::vector<double>{0.8})] = 0;
    backend.ints[{LINE_HANDLE, prop::OVERLOAD_DURATION}] = 60;
    backend.evaluations[std::make_tuple(ObjectHandle(LINE_HANDLE), prop::CHECK_PERMANENT_LIMIT,
                                        std::vector<double>{0, current()})] = 1;
    backend.evaluations[std::make_tuple(ObjectHandle(LINE_HANDLE), prop::CHECK_PERMANENT_LIMIT,
                                        std::vector<double>{1, 0.5, current()})] = 0;
    Line line(LINE_HANDLE, &backend);
    EXPECT_TRUE(line.isOverloaded());
    EXPECT_FALSE(line.isOverloaded(0.8));
    EXPECT_EQ(line.getOverloadDuration(), 60);
    EXPECT_TRUE(line.checkPermanentLimit1(LimitType::CURRENT));
    EXPECT_TRUE(line.checkPermanentLimit(TwoSides::ONE, LimitType::CURRENT));
    EXPECT_FALSE(line.checkPermanentLimit2(0.5, LimitType::CURRENT));
}

TEST_F(OverloadsTest, BranchTemporaryLimits) {
    backend.addResults[CHECK_HANDLE] = OVERLOAD_HANDLE;
    Line line(LINE_HANDLE, &backend);
    iidm::optional<Overload> overload = line.checkTemporaryLimits1(LimitType::CURRENT);

    // The check: started on the line, its side and type set (no limit reduction), then run
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(LINE_HANDLE), prop::CHECK_TEMPORARY_LIMITS));
    EXPECT_EQ((backend.ints[{CHECK_HANDLE, prop::TERMINAL_SIDE}]), 0);
    EXPECT_EQ((backend.ints[{CHECK_HANDLE, prop::LL_LIMIT_TYPE}]), static_cast<int>(LimitType::CURRENT));
    EXPECT_EQ(backend.doubles.count({CHECK_HANDLE, prop::LIMIT_REDUCTION}), 0u);
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(CHECK_HANDLE), prop::ACT_ADD));

    ASSERT_TRUE(overload);
    EXPECT_EQ(overload->getTemporaryLimit().getName(), "TL1");
    EXPECT_DOUBLE_EQ(overload->getPreviousLimit(), 900.0);
    EXPECT_EQ(overload->getPreviousLimitName(), "TL10");
    EXPECT_EQ(overload->getOperationalLimitsGroupId(), "DEFAULT");
    EXPECT_DOUBLE_EQ(overload->getLimitScalingCoefficient(), 1.0);

    // None found
    EXPECT_FALSE(line.checkTemporaryLimits(TwoSides::TWO, 0.9, LimitType::CURRENT));
    EXPECT_EQ((backend.ints[{CHECK_HANDLE + 1, prop::TERMINAL_SIDE}]), 1);
    EXPECT_DOUBLE_EQ((backend.doubles[{CHECK_HANDLE + 1, prop::LIMIT_REDUCTION}]), 0.9);
}

TEST_F(OverloadsTest, BranchAllTemporaryLimits) {
    backend.addResults[CHECK_HANDLE] = LIST_HANDLE;
    backend.children[{LIST_HANDLE, prop::ELEMENTS}] = {OVERLOAD_HANDLE};
    Line line(LINE_HANDLE, &backend);
    std::vector<Overload> overloads = line.checkAllTemporaryLimits(TwoSides::ONE, 0.9, LimitType::CURRENT);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(LINE_HANDLE), prop::CHECK_ALL_TEMPORARY_LIMITS));
    ASSERT_EQ(overloads.size(), 1u);
    EXPECT_EQ(overloads[0].getPreviousLimitName(), "TL10");
}

TEST_F(OverloadsTest, ThreeWindingsTransformer) {
    backend.bools[{T3_HANDLE, prop::OVERLOADED}] = true;
    backend.ints[{T3_HANDLE, prop::OVERLOAD_DURATION}] = 1200;
    backend.evaluations[std::make_tuple(ObjectHandle(T3_HANDLE), prop::CHECK_PERMANENT_LIMIT,
                                        std::vector<double>{2, current()})] = 1;
    backend.evaluations[std::make_tuple(ObjectHandle(T3_HANDLE), prop::CHECK_PERMANENT_LIMIT,
                                        std::vector<double>{0, current()})] = 0;
    backend.addResults[CHECK_HANDLE] = OVERLOAD_HANDLE;
    ThreeWindingsTransformer t3(T3_HANDLE, &backend);
    EXPECT_TRUE(t3.isOverloaded());
    EXPECT_EQ(t3.getOverloadDuration(), 1200);
    EXPECT_TRUE(t3.checkPermanentLimit3(LimitType::CURRENT));
    EXPECT_FALSE(t3.checkPermanentLimit1(LimitType::CURRENT));
    iidm::optional<Overload> overload = t3.checkTemporaryLimits3(LimitType::CURRENT);
    ASSERT_TRUE(overload);
    EXPECT_EQ((backend.ints[{CHECK_HANDLE, prop::TERMINAL_SIDE}]), 2);
    EXPECT_TRUE(t3.checkAllTemporaryLimits(ThreeSides::TWO, LimitType::CURRENT).empty());
}
