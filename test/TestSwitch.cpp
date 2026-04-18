#include <gtest/gtest.h>
#include <iidm/Switch.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class SwitchTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle SW_HANDLE = 50;

    void SetUp() override {
        backend.strings[{SW_HANDLE, prop::ID}]   = "SW1";
        backend.strings[{SW_HANDLE, prop::NAME}]  = "Switch 1";
        backend.bools  [{SW_HANDLE, prop::SW_OPEN}]     = false;
        backend.bools  [{SW_HANDLE, prop::SW_RETAINED}] = true;
        backend.ints   [{SW_HANDLE, prop::SW_KIND}]     = 0; // BREAKER
    }
};

TEST_F(SwitchTest, GetId) {
    Switch sw(SW_HANDLE, &backend);
    EXPECT_EQ(sw.getId(), "SW1");
}

TEST_F(SwitchTest, GetName) {
    Switch sw(SW_HANDLE, &backend);
    EXPECT_EQ(sw.getName(), "Switch 1");
}

TEST_F(SwitchTest, IsOpenAndSetOpen) {
    Switch sw(SW_HANDLE, &backend);
    EXPECT_FALSE(sw.isOpen());
    sw.setOpen(true);
    bool open = backend.bools[{SW_HANDLE, prop::SW_OPEN}];
    EXPECT_TRUE(open);
}

TEST_F(SwitchTest, IsRetainedAndSetRetained) {
    Switch sw(SW_HANDLE, &backend);
    EXPECT_TRUE(sw.isRetained());
    sw.setRetained(false);
    bool retained = backend.bools[{SW_HANDLE, prop::SW_RETAINED}];
    EXPECT_FALSE(retained);
}

TEST_F(SwitchTest, GetKind) {
    Switch sw(SW_HANDLE, &backend);
    EXPECT_EQ(sw.getKind(), SwitchKind::BREAKER);
}

TEST_F(SwitchTest, FluentSetter) {
    Switch sw(SW_HANDLE, &backend);
    sw.setOpen(true).setRetained(false);
    bool open     = backend.bools[{SW_HANDLE, prop::SW_OPEN}];
    bool retained = backend.bools[{SW_HANDLE, prop::SW_RETAINED}];
    EXPECT_TRUE(open);
    EXPECT_FALSE(retained);
}

TEST_F(SwitchTest, IsValid) {
    Switch valid(SW_HANDLE, &backend);
    Switch invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(SwitchTest, Equality) {
    Switch a(SW_HANDLE, &backend);
    Switch b(SW_HANDLE, &backend);
    Switch c(99, &backend);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}
