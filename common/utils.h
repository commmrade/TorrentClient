#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <span>

namespace utils {
double ceilTwoAfterComa(double number);
QString bytesToHigher(const std::uint64_t bytes);
QString bytesToHigherPerSec(const std::uint64_t bytes);

QString secsToFormattedTime(std::int64_t secs);

QString toHex(std::span<const char> data, bool to_upper = false);

}

#endif // UTILS_H
