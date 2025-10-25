#ifndef TORRENTHANDLE_H
#define TORRENTHANDLE_H
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>
#include <QString>
#include <QDebug>
#include "category.h"

class TorrentHandle
{
    lt::torrent_handle m_handle;

    QString m_category{Categories::RUNNING};
    // QStringList m_categories; // TODO: Maybe use a list

  public:
    void resetCategory();
    explicit TorrentHandle(lt::torrent_handle handle);
    TorrentHandle() = default;

    std::uint32_t id() const { return m_handle.id(); }

    const lt::torrent_handle &handle() const { return m_handle; }

    void connectToPeer(const boost::asio::ip::tcp::endpoint &ep);

    bool isNeedSaveData() const { return m_handle.need_save_resume_data(); }
    bool isValid() const { return m_handle.is_valid(); }

    std::vector<lt::announce_entry> getTrackers() const;
    std::vector<lt::peer_info>      getPeerInfo() const;

    QString bestHashAsString() const;
    QString hashV1AsString() const;

    lt::sha1_hash   hashV1() const { return m_handle.info_hashes().v1; }
    lt::sha256_hash hashV2() const { return m_handle.info_hashes().v2; }

    lt::sha1_hash bestHash() const
    { // may return truncated sha256
        return m_handle.info_hashes().get_best();
    }

    void setFilePriority(lt::file_index_t index, lt::download_priority_t priority);
    void renameFile(lt::file_index_t index, const QString &newName);
    void setDownloadLimit(int newLimit);
    int  getDownloadLimit() const;
    void setUploadLimit(int newLimit);
    int  getUploadLimit() const;
    void moveStorage(const QString &newPath);
    int getMaxConn() const;
    void setMaxConn(int value);

    void pause();
    bool isPaused() const
    {
        bool IsPaused =
            (m_handle.flags() & (lt::torrent_flags::paused)) == lt::torrent_flags::paused ? true
                                                                                          : false;
        return IsPaused;
    }
    void resume();

    void saveResumeData()
    {
        m_handle.save_resume_data(lt::torrent_handle::save_info_dict);
    }

    std::uint64_t activeDur() { return m_handle.status().active_duration.count(); }

    void    setCategory(const QString &category) { m_category = category; }
    QString getCategory() const { return m_category; }
};

#endif // TORRENTHANDLE_H
