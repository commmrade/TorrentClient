#ifndef TRACKER_H
#define TRACKER_H
#include <QString>

struct Tracker {
    QString url;
    std::uint8_t tier;
    bool isWorking;
    int seeds;
    int leeches;
    QString message;
    std::int32_t nextAnnounce;
    std::int32_t minAnnounce;
};

enum class TrackerFields {
    URL = 0,
    TIER,
    STATUS,
    SEEDS,
    LEECHES,
    MESSAGE,
    NEXT_ANNOUNCE,
    MIN_ANNOUNCE
};

constexpr int TRACKER_FIELDS_COUNT = (int)TrackerFields::MIN_ANNOUNCE + 1;

#endif // TRACKER_H
