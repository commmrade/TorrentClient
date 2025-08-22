#include "utils.h"
#include <cmath>

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
