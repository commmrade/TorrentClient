#ifndef TORRENTINFO_H
#define TORRENTINFO_H
#include <cstdint>
#include <QString>

struct InternetInfo {
    std::uint64_t activeTime;
    std::uint64_t downloaded;
    std::uint64_t downSpeed;
    std::uint64_t downLimit;

    std::int64_t eta; // signed for -1

    std::uint64_t uploaded;
    std::uint64_t upSpeed;
    std::uint64_t upLimit;

    int connections;
    int seeds;
    int peers;
};

struct TorrentInfo {
    std::uint64_t size;
    std::uint64_t startTime;

    std::optional<std::uint64_t> completedTime; // -1 for not completed yet

    QString hashBest;
    QString savePath;
    QString comment;

    int piecesCount;
    std::uint64_t pieceSize;
};

#endif // TORRENTINFO_H
