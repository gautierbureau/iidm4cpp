#include <gtest/gtest.h>
#include <iidm/Enums.h>

using namespace iidm;

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
