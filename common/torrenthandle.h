#ifndef TORRENTHANDLE_H
#define TORRENTHANDLE_H
#include <libtorrent/torrent_handle.hpp>
#include <chrono>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/hex.hpp>
#include <QString>
#include <QDebug>
// #include <libtorrent/libtorrent.hpp>

class TorrentHandle
{
    lt::torrent_handle m_handle;
public:
    explicit TorrentHandle(lt::torrent_handle handle) : m_handle(handle) {};
    TorrentHandle() = default;

    std::uint32_t id() const {
        return m_handle.id();
    }

    const lt::torrent_handle& handle() const {
        return m_handle;
    }

    bool isNeedSaveData() const {
        return m_handle.need_save_resume_data();
    }
    bool isValid() const {
        return m_handle.is_valid();
    }

    std::vector<lt::announce_entry> getTrackers() const;

    std::vector<lt::peer_info> getPeerInfo() const {
        std::vector<lt::peer_info> peers;
        m_handle.get_peer_info(peers);
        return peers;
    }

    QString bestHashAsString() const {
        return QString::fromStdString(lt::aux::to_hex(m_handle.info_hashes().get_best()));
    }
    lt::sha1_hash bestHash() const {
        return m_handle.info_hashes().get_best();
    }

    void pause() {
        m_handle.unset_flags(lt::torrent_flags::auto_managed); // We need this so libtorrent wont auto resume torrent handle
        m_handle.pause();
    }
    bool isPaused() const {
        bool IsPaused = (m_handle.flags() & (lt::torrent_flags::paused)) == lt::torrent_flags::paused ? true : false;
        return IsPaused;
    }
    void resume() {
        m_handle.resume();
    }

    void saveResumeData() {
        m_handle.save_resume_data();
    }

    std::uint64_t activeDur() {
        return m_handle.status().active_duration.count();
    }
};

#endif // TORRENTHANDLE_H
