#ifndef TORRENTINFO_H
#define TORRENTINFO_H

#include <cstdint>
#include <QString>

struct InternetInfo
{
    std::int64_t activeTime;
    std::int64_t downloaded;
    int          downSpeed;
    int          downLimit;

    std::int64_t eta; // signed for -1

    std::int64_t uploaded;
    int          upSpeed;
    int          upLimit;

    double ratio;

    int connections;
    int seeds;
    int peers;
};

struct TorrentInfo
{
    std::int64_t size;
    std::int64_t startTime;

    std::int64_t completedTime; // -1 for not completed yet

    // QString hashBest;
    QString hashV1;
    QString hashV2;

    QString savePath;
    QString comment;

    int piecesCount;
    int pieceSize;
};

#endif // TORRENTINFO_H
