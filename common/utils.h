#ifndef UTILS_H
#define UTILS_H
#include <QString>

double ceilTwoAfterComa(double number);
QString bytesToHigher(const std::uint64_t bytes);
QString bytesToHigherPerSec(const std::uint64_t bytes);

QString secsToFormattedTime(std::int64_t secs);

#endif // UTILS_H
