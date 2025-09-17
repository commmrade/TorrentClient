#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <span>

namespace utils {
inline constexpr int SECONDS_IN_DAY = 86400;
inline constexpr int SECONDS_IN_HOUR = 3600;
inline constexpr int SECONDS_IN_MINUTE = 60;
inline constexpr int SECONDS_IN_WEEK = 604800;
inline constexpr int SECONDS_IN_YEAR = 31449600;

inline constexpr auto BYTES_IN_KB = 1024;
inline constexpr auto BYTES_IN_MB = 1024 * 1024;
inline constexpr auto BYTES_IN_GB = 1024 * 1024 * 1024;

inline constexpr auto DBYTES_IN_KB = 1024.0;
inline constexpr auto DBYTES_IN_MB = 1024.0 * 1024.0;
inline constexpr auto DBYTES_IN_GB = 1024.0 * 1024.0 * 1024.0;


double ceilTwoAfterComa(double number);
QString bytesToHigher(const std::uint64_t bytes);
QString bytesToHigherPerSec(const std::uint64_t bytes);

QString secsToFormattedTime(std::int64_t secs);

QString toHex(std::span<const char> data, bool to_upper = false);

}

#endif // UTILS_H
