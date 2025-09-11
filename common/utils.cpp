#include "utils.h"
#include <cmath>



namespace utils {

constexpr int SECONDS_IN_DAY = 86400;
constexpr int SECONDS_IN_HOUR = 3600;
constexpr int SECONDS_IN_MINUTE = 60;
constexpr int SECONDS_IN_WEEK = 604800;
constexpr int SECONDS_IN_YEAR = 31449600;
double ceilTwoAfterComa(double number) {
    return std::ceil(number * 100.0) / 100.0;
}

QString bytesToHigher(const std::uint64_t bytes)
{
    QString sizeStr{QString::number(bytes) + " B"};
    if (bytes < 1024 * 1024) { // < mb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / 1024.0)) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) { // < gb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / 1024.0 / 1024.0)) + " MB";
    } else if (bytes >= 1024 * 1024 * 1024) { // >= gb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / 1024.0 / 1024.0 / 1024.0)) + " GB";
    }
    return sizeStr;
}

QString bytesToHigherPerSec(const std::uint64_t bytes)
{
    QString sizeStr{QString::number(bytes) + " B/s"};
    if (bytes < 1024 * 1024) { // < mb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / 1024.0)) + " KB/s";
    } else if (bytes < 1024 * 1024 * 1024) { // < gb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / 1024.0 / 1024.0)) + " MB/s";
    } else if (bytes >= 1024 * 1024 * 1024) { // >= gb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / 1024.0 / 1024.0 / 1024.0)) + " GB/s";
    }
    return sizeStr;
}

QString secsToFormattedTime(std::int64_t secs)
{
    if (secs == -1) return QString("infinity");

    QString etaStr;
    auto years = secs / SECONDS_IN_YEAR;
    if (years) {
        etaStr += QString::number(years) + " y ";
    }
    auto weeks = secs / SECONDS_IN_WEEK;
    if (weeks) {
        etaStr += QString::number(weeks) + " w ";
    }
    auto days = secs / SECONDS_IN_DAY;
    if (days) {
        etaStr += QString::number(days) + " d ";
    }

    auto hrs = secs % SECONDS_IN_YEAR % SECONDS_IN_WEEK % SECONDS_IN_DAY / SECONDS_IN_HOUR;
    auto mins = secs % SECONDS_IN_HOUR / SECONDS_IN_MINUTE;
    auto seconds = secs % SECONDS_IN_MINUTE;
    etaStr += QString("%1:%2:%3").arg(hrs).arg(mins).arg(seconds);

    return etaStr;
}

QString toHex(std::span<const char> data, bool to_upper /* = false */)
{
    QString result;
    result.reserve(data.size() * 2);

    uint8_t into_letter = to_upper ? 55 : 87; // (65 ('A') - 10 ('hex letters')) and  (97 ('a) - 10 ('hex letters'))

    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.data());
    for (auto i = 0; i < data.size(); ++i) {
        uint8_t byte_first = (bytes[i] >> 4);
        uint8_t byte_second = (0x0F) & bytes[i];
        result += (byte_first < 10) ? static_cast<char>(byte_first + 48) : static_cast<char>(byte_first + into_letter);
        result += (byte_second < 10) ? static_cast<char>(byte_second + 48) : static_cast<char>(byte_second + into_letter);
    }
    return result;
}

} // namespace utils
