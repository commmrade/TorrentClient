#ifndef PEER_H
#define PEER_H
#include <QString>

struct Peer {
    QString country;
    QString ip;
    std::uint16_t port;
    QString connectionType;
    QString client;
    float progress;
    std::uint64_t upSpeed;
    std::uint64_t downSpeed;
    std::uint64_t downloaded;
    std::uint64_t uploaded;
};

enum class PeerFields {
    COUNTRY = 0,
    IP,
    PORT,
    CONNECTION,
    CLIENT,
    PROGRESS,
    UP_SPEED,
    DOWN_SPEED,
    DOWNLOADED,
    UPLOADED
};

constexpr int PEER_FIELD_COUNT = (int)PeerFields::UPLOADED + 1;

#endif // PEER_H
