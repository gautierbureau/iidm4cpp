#include <gtest/gtest.h>
#include <iidm/Enums.h>
#include <iidm/Substation.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

TEST(EnumsTest, GetCountryNameFR) {
    EXPECT_EQ(getCountryName(Country::FR), "FR");
}

TEST(EnumsTest, GetCountryNameDE) {
    EXPECT_EQ(getCountryName(Country::DE), "DE");
}

TEST(EnumsTest, GetCountryNameUS) {
    EXPECT_EQ(getCountryName(Country::US), "US");
}

TEST(EnumsTest, GetCountryNameGB) {
    EXPECT_EQ(getCountryName(Country::GB), "GB");
}

TEST(EnumsTest, GetCountryNameZW) {
    EXPECT_EQ(getCountryName(Country::ZW), "ZW");
}

TEST(EnumsTest, GetCountryNameAF) {
    EXPECT_EQ(getCountryName(Country::AF), "AF");
}

TEST(EnumsTest, CountryFromNameRoundTrip) {
    EXPECT_EQ(countryFromName("FR"),  Country::FR);
    EXPECT_EQ(countryFromName("DE"),  Country::DE);
    EXPECT_EQ(countryFromName("AF"),  Country::AF);
    EXPECT_EQ(countryFromName("ZW"),  Country::ZW);
}

TEST(EnumsTest, CountryFromNameEmptyIsNullopt) {
    EXPECT_FALSE(countryFromName("").has_value());
}

// ── Substation::getCountry via getString ──────────────────────────────────────

TEST(EnumsTest, SubstationGetCountryPresent) {
    MockBackend b;
    static constexpr ObjectHandle H = 1;
    b.strings[{H, prop::COUNTRY}] = "AF";
    Substation sub(H, &b);
    auto c = sub.getCountry();
    ASSERT_TRUE(c.has_value());
    EXPECT_EQ(*c, Country::AF);
}

TEST(EnumsTest, SubstationGetCountryAbsent) {
    MockBackend b;
    static constexpr ObjectHandle H = 1;
    // no COUNTRY entry → getString returns "" → nullopt
    Substation sub(H, &b);
    EXPECT_FALSE(sub.getCountry().has_value());
}
