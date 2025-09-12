#include "torrenthandle.h"


std::vector<libtorrent::announce_entry> TorrentHandle::getTrackers() const
{
    return m_handle.trackers();
}

void TorrentHandle::setFilePriority(libtorrent::file_index_t index, libtorrent::download_priority_t priority) {
    m_handle.file_priority(index, priority);
}
