#include <gtest/gtest.h>
#include <iidm/Substation.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class SubstationTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle SUB_H = 1;
    static constexpr ObjectHandle VL1_H = 10;
    static constexpr ObjectHandle VL2_H = 11;

    void SetUp() override {
        backend.strings[{SUB_H, prop::ID}]      = "SUB1";
        backend.strings[{SUB_H, prop::NAME}]     = "Substation 1";
        backend.strings[{SUB_H, prop::COUNTRY}]  = "FR";
    }
};

TEST_F(SubstationTest, GetId) {
    Substation sub(SUB_H, &backend);
    EXPECT_EQ(sub.getId(), "SUB1");
}

TEST_F(SubstationTest, GetName) {
    Substation sub(SUB_H, &backend);
    EXPECT_EQ(sub.getName(), "Substation 1");
}

TEST_F(SubstationTest, GetCountryPresent) {
    Substation sub(SUB_H, &backend);
    auto c = sub.getCountry();
    ASSERT_TRUE(c.has_value());
    EXPECT_EQ(*c, Country::FR);
}

TEST_F(SubstationTest, GetCountryAbsent) {
    MockBackend b;
    b.strings[{SUB_H, prop::ID}] = "SUB2";
    Substation sub(SUB_H, &b);
    EXPECT_FALSE(sub.getCountry().has_value());
}

TEST_F(SubstationTest, IsValid) {
    Substation valid(SUB_H, &backend);
    Substation invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(SubstationTest, GetVoltageLevels) {
    backend.children[{SUB_H, prop::VOLTAGE_LEVEL}] = {VL1_H, VL2_H};
    backend.strings[{VL1_H, prop::ID}] = "VL1";
    backend.strings[{VL2_H, prop::ID}] = "VL2";
    Substation sub(SUB_H, &backend);
    auto vls = sub.getVoltageLevels();
    ASSERT_EQ(vls.size(), 2u);
    EXPECT_EQ(vls[0].getId(), "VL1");
    EXPECT_EQ(vls[1].getId(), "VL2");
}

TEST_F(SubstationTest, GetVoltageLevelsEmpty) {
    Substation sub(SUB_H, &backend);
    EXPECT_TRUE(sub.getVoltageLevels().empty());
}

TEST_F(SubstationTest, Equality) {
    Substation a(SUB_H, &backend);
    Substation b(SUB_H, &backend);
    Substation c(42,    &backend);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}
