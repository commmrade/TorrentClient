#ifndef TORRENT_H
#define TORRENT_H


#include <QString>
#include <libtorrent/libtorrent.hpp>

struct Torrent {
    std::uint32_t id;
    QString name;
    QString size;
    double progress; // 0.0% to 100.0%
    QString status;
    int seeds;
    int peers;
    QString downSpeed;
    QString upSpeed;
};

enum TorrentsFields {
    ID = 0,
    NAME,
    SIZE,
    PROGRESS,
    STATUS,
    SEEDS,
    PEERS,
    DOWN_SPEED,
    UP_SPEED
};
constexpr int TORRENT_FIELD_COUNT = TorrentsFields::UP_SPEED + 1;

inline QString torrentStateToString(lt::torrent_status::state_t state) {
    using lt::torrent_status;

    switch (state) {
    case torrent_status::checking_files:
        return "Checking files";
    case torrent_status::downloading_metadata:
        return "Downloading metadata";
    case torrent_status::downloading:
        return "Downloading";
    case torrent_status::finished:
        return "Finished";
    case torrent_status::seeding:
        return "Seeding";
    case torrent_status::checking_resume_data:
        return "Checking resume data";
    default:
        return "Unknown";
    }
}


#endif // TORRENT_H
