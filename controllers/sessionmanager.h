#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <vector>
#include <libtorrent/session.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/aux_/session_call.hpp>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include "dirs.h"
#include "torrenthandle.h"
#include "tracker.h"
#include "file.h"

constexpr const char *SESSION_FILENAME = ".session";

struct Torrent;
struct TorrentInfo;
struct InternetInfo;
// struct Tracker;

class SessionManager : public QObject
{
    Q_OBJECT

    std::unique_ptr<lt::session> m_session;
    void                         loadSessionSettingsFromSettings(lt::session_params &sessParams);
    QHash<std::uint32_t, TorrentHandle> m_torrentHandles;

    QTimer m_alertTimer;
    QTimer m_resumeDataTimer;

    // std::int64_t m_currentTorrentId{-1};
    std::optional<std::uint32_t> m_currentTorrentId{std::nullopt};

    std::int64_t lastSessionRecvPayloadBytes{0};
    std::int64_t lastSessionUploadPayloadBytes{0};

    explicit SessionManager(QObject *parent = nullptr);

  public:
    ~SessionManager();

    static SessionManager &instance()
    {
        static SessionManager sessionManager{nullptr};
        return sessionManager;
    }

    const TorrentHandle getTorrentHandle(const std::uint32_t id) const;
    bool                addTorrentByFilename(QStringView filepath, QStringView outputDir);
    bool                addTorrentByMagnet(QString magnetURI, QStringView outputDir);
    bool                addTorrentByTorrentInfo(std::shared_ptr<const lt::torrent_info> ti,
                                                const QList<lt::download_priority_t>   &filePriorities,
                                                QStringView                             outputDir);

    bool isTorrentPaused(const std::uint32_t) const;
    void pauseTorrent(const std::uint32_t id);
    void resumeTorrent(const std::uint32_t id);
    bool removeTorrent(const std::uint32_t id, bool removeWithContents);
    void setTorrentDownloadLimit(const std::uint32_t, int newLimit);
    void setTorrentUploadLimit(const std::uint32_t, int newLimit);
    void setTorrentSavePath(const std::uint32_t id, const QString &newPath);

    void loadResumes();

    // Managing session
    void setDownloadLimit(int value);
    void setUploadLimit(int value);

    // Files
    void changeFilePriority(std::uint32_t id, int fileIndex, int priority); // TODO: Impl
    void renameFile(std::uint32_t id, int fileIndex, const QString &newName);

    // Peer
    void banPeers(const QList<QPair<QString, unsigned short>> &bannablePeers);
    void addPeerToTorrent(std::uint32_t torrentId, const boost::asio::ip::tcp::endpoint &ep);
    void addPeersToTorrent(std::uint32_t                                torrentId,
                           const QList<boost::asio::ip::tcp::endpoint> &eps);

    // Notice: this kinda feels wrong to track current torrent in here, so maybe i can come up with
    // something better
    void                         setCurrentTorrentId(std::optional<std::uint32_t> value);
    std::optional<std::uint32_t> getCurrentTorrentId() const { return m_currentTorrentId; }

    // Utils
    void forceUpdateProperties();

  private:
    lt::session_params loadSessionParams();

    // Utils
    void emitClearSignals();

    // Event loop and alert functions
    void eventLoop();

    void updateProperties();
    void updatePeersProp(TorrentHandle &handle);
    void updateTrackersProp(TorrentHandle &handle);
    void updateFilesProp(TorrentHandle &handle);
    void updateUrlProp(TorrentHandle &handle);
    void updateTorrent(TorrentHandle &handle, const lt::torrent_status &status);

    void updateGeneralProperty(const lt::torrent_handle &handle);
    void handleFinishedAlert(lt::torrent_finished_alert *alert);
    void handleStateUpdateAlert(lt::state_update_alert *alert);
    void handleMetadataReceived(lt::metadata_received_alert *alert);
    void handleResumeDataAlert(lt::save_resume_data_alert *alert);
    void handleAddTorrentAlert(lt::add_torrent_alert *alert);
    void handleSessionStatsAlert(lt::session_stats_alert *alert);
    void handleTorrentErrorAlert(lt::torrent_error_alert *alert);
    void handleStorageMoveFailedAlert(lt::storage_moved_failed_alert *alert);

    void saveResumes();
    bool addTorrent(lt::add_torrent_params params);
    void handleStatusUpdate(const lt::torrent_status &status, const lt::torrent_handle &handle);

    bool isTorrentExists(const lt::sha1_hash &hash) const;
  signals:
    void torrentAdded(const Torrent &torrent);
    void torrentUpdated(const Torrent &torrent);
    void torrentFinished(const std::uint32_t id, const lt::torrent_status &status);
    void torrentDeleted(const std::uint32_t id);
    // void torrentStorageMoveFailed(const QString& message);
    void torrentStorageMoveFailed(const QString &message, const QString &whereTo);
    // or
    // void error(const QString& type, const QString& message);

    void peerInfo(const std::uint32_t id, const std::vector<lt::peer_info> &peers);
    void clearPeerInfo();

    void generalInfo(const TorrentInfo &tInfo, const InternetInfo &iInfo);
    void clearGeneralInfo();

    void trackersInfo(const QList<Tracker> &trackers);
    void clearTrackers();

    void urlSeedsInfo(const std::set<std::string> &);
    void clearUrlSeeds();

    void pieceBarInfo(const lt::typed_bitfield<lt::piece_index_t> &pieces,
                      const std::vector<int>                      &downloadingPiecesIdx);

    void chartPoint(int download, int upload);

    void filesInfo(const QList<File> &files);
    void clearFiles();
};

namespace detail
{
void writeTorrentFile(std::shared_ptr<const lt::torrent_info> ti);
void saveResumeData(std::shared_ptr<const lt::torrent_info> ti, const std::vector<char> &buf);
std::vector<char> readFile(const char *filename);
} // namespace detail

#endif // SESSIONMANAGER_H
