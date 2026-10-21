#include <gtest/gtest.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/ActivePowerControlAdder.h>
#include <iidm/Battery.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class BatteryTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle BAT_HANDLE      = 30;
    static constexpr ObjectHandle TERM_HANDLE     = 31;
    static constexpr ObjectHandle BAT_APC_HANDLE  = 32; // ActivePowerControl extension object

    void SetUp() override {
        backend.strings[{BAT_HANDLE, prop::ID}]    = "BAT1";
        backend.strings[{BAT_HANDLE, prop::NAME}]   = "Battery 1";
        backend.doubles[{BAT_HANDLE, prop::BAT_TARGET_P}] = 50.0;
        backend.doubles[{BAT_HANDLE, prop::LOCAL_TARGET_Q}] = 10.0;
        backend.doubles[{BAT_HANDLE, prop::BAT_MIN_P}]    = -100.0;
        backend.doubles[{BAT_HANDLE, prop::BAT_MAX_P}]    = 100.0;
        backend.related[{BAT_HANDLE, prop::REL_TERMINAL}] = TERM_HANDLE;
    }
};

TEST_F(BatteryTest, LocalTargetQGetSet) {
    Battery bat(BAT_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(bat.getLocalTargetQ(), 10.0);
    bat.setLocalTargetQ(15.0).setTargetP(1.0); // chains with Battery's own setters
    double tq = backend.doubles[{BAT_HANDLE, prop::LOCAL_TARGET_Q}];
    EXPECT_DOUBLE_EQ(tq, 15.0);
}

// ── Battery reactive limits ────────────────────────────────────────────────

// Same reactive limits objects, and same codes, as on a generator
TEST_F(BatteryTest, ReactiveLimits) {
    const ObjectHandle limitsHandle = 33;
    backend.related[{BAT_HANDLE, prop::REACTIVE_LIMITS}]         = limitsHandle;
    backend.related[{BAT_HANDLE, prop::MIN_MAX_REACTIVE_LIMITS}] = limitsHandle;
    backend.ints   [{limitsHandle, prop::RL_KIND}]  = static_cast<int>(ReactiveLimitsKind::MIN_MAX);
    backend.doubles[{limitsHandle, prop::RL_MIN_Q}] = -80.0;
    backend.doubles[{limitsHandle, prop::RL_MAX_Q}] =  80.0;
    Battery bat(BAT_HANDLE, &backend);
    EXPECT_EQ(bat.getReactiveLimits().getKind(), ReactiveLimitsKind::MIN_MAX);
    MinMaxReactiveLimits rl = bat.getReactiveLimits<MinMaxReactiveLimits>();
    EXPECT_DOUBLE_EQ(rl.getMinQ(), -80.0);
    EXPECT_DOUBLE_EQ(rl.getMaxQ(),  80.0);
}

// ── Battery ActivePowerControl extension ──────────────────────────────────

TEST_F(BatteryTest, SetMinMaxPAndSolvedValues) {
    Battery bat(BAT_HANDLE, &backend);
    EXPECT_EQ(&bat.setMinP(-20.0).setMaxP(30.0), &bat);
    EXPECT_DOUBLE_EQ(bat.getMinP(), -20.0);
    EXPECT_DOUBLE_EQ(bat.getMaxP(), 30.0);
    bat.applySolvedValues();
    bat.setTargetPToP();
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(BAT_HANDLE), prop::ACT_APPLY_SOLVED_VALUES));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(BAT_HANDLE), prop::ACT_SET_TARGET_P_TO_P));
}
