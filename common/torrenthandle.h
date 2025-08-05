#ifndef TORRENTHANDLE_H
#define TORRENTHANDLE_H
#include <libtorrent/torrent_handle.hpp>

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

    // TODO: add default flags and etc
    bool isNeedSaveData() const {
        return m_handle.need_save_resume_data();
    }
    bool isValid() const {
        return m_handle.is_valid();
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
};

#endif // TORRENTHANDLE_H
