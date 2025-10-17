#include "utils.h"
#include <cmath>
#include <qdebug.h>

namespace utils
{

double ceilTwoAfterComa(double number) { return std::ceil(number * 100.0) / 100.0; }

QString bytesToHigher(const std::uint64_t bytes)
{
    QString sizeStr{QString::number(bytes) + " B"};
    if (bytes < BYTES_IN_KB)
    {
    }
    else if (bytes < BYTES_IN_MB)
    { // < mb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / DBYTES_IN_KB)) + " KB";
    }
    else if (bytes < BYTES_IN_GB)
    { // < gb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / DBYTES_IN_MB)) + " MB";
    }
    else
    { // >= gb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / DBYTES_IN_GB)) + " GB";
    }
    return sizeStr;
}

QString bytesToHigherPerSec(const std::uint64_t bytes)
{
    QString sizeStr{QString::number(bytes) + " B/s"};
    if (bytes < BYTES_IN_KB)
    {
    }
    else if (bytes < BYTES_IN_MB)
    { // < mb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / DBYTES_IN_KB)) + " KB/s";
    }
    else if (bytes < BYTES_IN_GB)
    { // < gb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / DBYTES_IN_MB)) + " MB/s";
    }
    else
    { // >= gb
        sizeStr = QString::number(ceilTwoAfterComa(bytes / DBYTES_IN_GB)) + " GB/s";
    }
    return sizeStr;
}

QString secsToFormattedTime(std::int64_t secs)
{
    if (secs == -1 || secs > SECONDS_IN_YEAR)
        return QString("infinity");

    QString etaStr;
    auto months = secs / SECONDS_IN_MONTH;
    if (months) {
        etaStr += QString::number(months) + " m ";
    }
    secs -= months * SECONDS_IN_MONTH;

    auto weeks = secs / SECONDS_IN_WEEK;
    if (weeks)
    {
        etaStr += QString::number(weeks) + " w ";
    }
    secs -= weeks * SECONDS_IN_WEEK;

    auto days = secs / SECONDS_IN_DAY;
    if (days)
    {
        etaStr += QString::number(days) + " d ";
    }
    secs -= days * SECONDS_IN_DAY;

    auto hrs     = secs / SECONDS_IN_HOUR;
    secs -= hrs * SECONDS_IN_HOUR;

    auto mins    = secs / SECONDS_IN_MINUTE;
    secs -= mins * SECONDS_IN_MINUTE;

    auto seconds = secs;
    etaStr += QString("%1:%2:%3").arg(hrs, 2, 10, '0').arg(mins, 2, 10, '0').arg(seconds, 2, 10, '0');
    return etaStr;
}

QString toHex(std::span<const char> data, bool to_upper /* = false */)
{
    QString result;
    result.reserve(data.size() * 2);

    uint8_t into_letter =
        to_upper ? 55 : 87; // (65 ('A') - 10 ('hex letters')) and  (97 ('a) - 10 ('hex letters'))

    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(data.data());
    for (auto i = 0; i < data.size(); ++i)
    {
        uint8_t byte_first  = (bytes[i] >> 4);
        uint8_t byte_second = (0x0F) & bytes[i];
        result += (byte_first < 10) ? static_cast<char>(byte_first + 48)
                                    : static_cast<char>(byte_first + into_letter);
        result += (byte_second < 10) ? static_cast<char>(byte_second + 48)
                                     : static_cast<char>(byte_second + into_letter);
    }
    return result;
}

} // namespace utils
