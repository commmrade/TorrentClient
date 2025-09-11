#ifndef TORRENT_H
#define TORRENT_H

#include <QString>
#include <libtorrent/torrent_status.hpp>

// All int fields are in bytes
struct Torrent {
    std::uint32_t id;
    QString name;
    // QString size;
    std::int64_t size;
    double progress; // 0.0% to 100.0%
    QString status;
    int seeds;
    int peers;
    // QString downSpeed;
    int downSpeed; // Int is sufficient enough
    // QString upSpeed;
    int upSpeed;
    // int eta; // In seconds
    std::int64_t eta; // signed for -1 if inf
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
    UP_SPEED,
    ETA,

};

constexpr int TORRENT_FIELD_COUNT = TorrentsFields::ETA + 1;

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
