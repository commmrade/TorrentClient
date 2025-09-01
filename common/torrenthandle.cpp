#include "torrenthandle.h"


std::vector<libtorrent::announce_entry> TorrentHandle::getTrackers() const
{
    // im gonna display:
    // 1. url
    // 2. tier
    // 3. status (if any endpoint is true, then true "working")
    // 4. seeds (scrape_completed)
    // 5. leeches (scrape_incomplete)
    // 6. message
    // const auto trackers = m_handle.trackers();

    // qDebug() << "Trackers size" << trackers.size();
    // for (const lt::announce_entry& tracker : trackers) {
    //     qDebug() << "URL" << tracker.url << "Trackerid" << tracker.trackerid << "Endpoint cnt" << tracker.endpoints.size() << "Verified" << tracker.verified << "tier" << tracker.tier;
    //     qDebug() << "-------";

    //     // Use protocol V1 for now
    //     for (const auto& endpoint : tracker.endpoints) {
    //         auto& ihash =  endpoint.info_hashes[lt::protocol_version::V1];
    //         // qDebug() << "
    //         // qDebug() << "Endpoint enabled" << endpoint.enabled;
    //         // for (const auto& ihash : endpoint.info_hashes) {
    //         //     qDebug() << "Leeches" << ihash.scrape_incomplete << "Seeds" << ihash.scrape_complete;
    //         // }
    //     }

    // }
    return m_handle.trackers();
}
