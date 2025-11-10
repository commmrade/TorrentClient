#include <gtest/gtest.h>
#include <span>
#include <cmath>
#include "../src/core/utils/utils.h"

using namespace utils;

TEST(UtilsTest, CeilTwoAfterComa_Works) {
    EXPECT_NEAR(ceilTwoAfterComa(1.2345), 1.24, 1e-9);
    EXPECT_NEAR(ceilTwoAfterComa(1.2), 1.20, 1e-9);
    EXPECT_NEAR(ceilTwoAfterComa(0.001), 0.01, 1e-9); // ceiling to two decimals
}

TEST(UtilsTest, BytesToHigher_FormatsSizesCorrectly) {
    EXPECT_EQ(bytesToHigher(500).toStdString(), "500 B");
    EXPECT_EQ(bytesToHigher(1024).toStdString(), "1 KB");
    EXPECT_EQ(bytesToHigher(1536).toStdString(), "1.5 KB");
    EXPECT_EQ(bytesToHigher(1024ULL * 1024ULL).toStdString(), "1 MB");
    // 5 MB + 123 bytes -> ~5.000117 -> ceilTwoAfterComa -> 5.01 MB
    EXPECT_EQ(bytesToHigher(5ULL * 1024ULL * 1024ULL + 123ULL).toStdString(), "5.01 MB");
    EXPECT_EQ(bytesToHigher(3ULL * 1024ULL * 1024ULL * 1024ULL).toStdString(), "3 GB");
}

TEST(UtilsTest, BytesToHigherPerSec_FormatsSpeedsCorrectly) {
    EXPECT_EQ(bytesToHigherPerSec(500).toStdString(), "500 B/s");
    EXPECT_EQ(bytesToHigherPerSec(2048).toStdString(), "2 KB/s");
    // 5 MB + 512 bytes -> ~5.000488 -> ceilTwoAfterComa -> 5.01 MB/s
    EXPECT_EQ(bytesToHigherPerSec(5ULL * 1024ULL * 1024ULL + 512ULL).toStdString(), "5.01 MB/s");
}

TEST(UtilsTest, SecsToFormattedTime_HandlesSpecialValuesAndDecomposition) {
    EXPECT_EQ(secsToFormattedTime(-1).toStdString(), "infinity");
    EXPECT_EQ(secsToFormattedTime(SECONDS_IN_YEAR + 1).toStdString(), "infinity");

    // 3661 = 1 hour, 1 minute, 1 second -> 01:01:01
    EXPECT_EQ(secsToFormattedTime(3661).toStdString(), "01:01:01");

    // Compose: 1 month + 2 weeks + 3 days + 4:05:06
    long long secs = 1LL * SECONDS_IN_MONTH + 2LL * SECONDS_IN_WEEK + 3LL * SECONDS_IN_DAY +
                     4LL * SECONDS_IN_HOUR + 5LL * SECONDS_IN_MINUTE + 6LL;
    EXPECT_EQ(secsToFormattedTime(secs).toStdString(), "1 m 2 w 3 d 04:05:06");
}

TEST(UtilsTest, ToHex_ProducesCorrectLowercaseAndUppercaseHexStrings) {
    {
        const char data[] = { (char)0x61, (char)0x62, (char)0x63 }; // 'a','b','c'
        auto s = std::span<const char>(data, 3);
        EXPECT_EQ(toHex(s).toStdString(), "616263");
    }
    {
        const char data[] = { (char)0xAF, (char)0x10 };
        auto s = std::span<const char>(data, 2);
        EXPECT_EQ(toHex(s, false).toStdString(), "af10");
        EXPECT_EQ(toHex(s, true).toStdString(), "AF10");
    }
    {
        const char data[] = {};
        auto s = std::span<const char>(data, 0);
        EXPECT_EQ(toHex(s).toStdString(), "");
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
