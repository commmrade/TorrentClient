#include "torrenthandle.h"


std::vector<libtorrent::announce_entry> TorrentHandle::getTrackers() const
{
    return m_handle.trackers();
}

void TorrentHandle::resetCategory()
{
    auto status = m_handle.status();
    if (status.is_seeding || status.is_finished) { // they are kinda the same
        m_category = "Seeding";
    } else if ((m_handle.flags() & (lt::torrent_flags::paused)) == lt::torrent_flags::paused ? true : false) {
        m_category = "Stopped";
    } else { // not seeding and not finished and not paused
        m_category = "Running";
    }
}

TorrentHandle::TorrentHandle(libtorrent::torrent_handle handle) : m_handle(handle) {
    resetCategory();
}

void TorrentHandle::setFilePriority(libtorrent::file_index_t index, libtorrent::download_priority_t priority) {
    m_handle.file_priority(index, priority);
}

void TorrentHandle::renameFile(libtorrent::file_index_t index, const QString& newName)
{
    // File is renamed asynchronously, so should probably get an alert
    // TODO: Handle alerts or not?
    m_handle.rename_file(index, newName.toStdString());
}
void TorrentHandle::pause() {
    setCategory("Stopped");
    m_handle.unset_flags(lt::torrent_flags::auto_managed); // We need this so libtorrent wont auto resume torrent handle
    m_handle.pause();
}

void TorrentHandle::resume() {
    resetCategory();
    m_handle.resume();
}
