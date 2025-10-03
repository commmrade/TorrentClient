#include "torrenthandle.h"
#include "category.h"
#include "utils.h"

std::vector<libtorrent::announce_entry> TorrentHandle::getTrackers() const
{
    return m_handle.trackers();
}

void TorrentHandle::resetCategory()
{
    auto status = m_handle.status();
    if (isPaused())
    {
        m_category = Categories::STOPPED;
    }
    else if (status.is_seeding || status.is_finished)
    { // they are kinda the same
        m_category = Categories::SEEDING;
    }
    else
    { // not seeding and not finished and not paused
        m_category = Categories::RUNNING;
    }
}

TorrentHandle::TorrentHandle(libtorrent::torrent_handle handle) : m_handle(handle)
{
    resetCategory();
}

void TorrentHandle::setFilePriority(libtorrent::file_index_t        index,
                                    libtorrent::download_priority_t priority)
{
    m_handle.file_priority(index, priority);
}

void TorrentHandle::renameFile(libtorrent::file_index_t index, const QString &newName)
{
    // File is renamed asynchronously, so should probably get an alert
    // TODO: Handle alerts or not?
    m_handle.rename_file(index, newName.toStdString());
}

void TorrentHandle::setDownloadLimit(int newLimit) { m_handle.set_download_limit(newLimit); }

int TorrentHandle::getDownloadLimit() const { return m_handle.download_limit(); }

void TorrentHandle::setUploadLimit(int newLimit) { m_handle.set_upload_limit(newLimit); }

int TorrentHandle::getUploadLimit() const { return m_handle.upload_limit(); }

void TorrentHandle::moveStorage(const QString &newPath)
{
    m_handle.move_storage(newPath.toStdString(), lt::move_flags_t::always_replace_files);
}
void TorrentHandle::pause()
{
    setCategory(Categories::STOPPED);
    m_handle.unset_flags(lt::torrent_flags::auto_managed); // We need this so libtorrent wont auto
                                                           // resume torrent handle
    m_handle.pause();
}

void TorrentHandle::resume()
{
    m_handle.resume();
    resetCategory();
}
void TorrentHandle::connectToPeer(const boost::asio::ip::tcp::endpoint &ep)
{
    m_handle.connect_peer(ep);
}

QString TorrentHandle::bestHashAsString() const
{ // truncates sha256
    return utils::toHex(m_handle.info_hashes().get_best().to_string());
}

QString TorrentHandle::hashV1AsString() const
{
    return utils::toHex(m_handle.info_hashes().v1.to_string());
}

std::vector<libtorrent::peer_info> TorrentHandle::getPeerInfo() const
{
    std::vector<lt::peer_info> peers;
    m_handle.get_peer_info(peers);
    return peers;
}
