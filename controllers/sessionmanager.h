#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <fstream>
#include <vector>
#include <libtorrent/session.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/aux_/session_call.hpp>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include "torrenthandle.h"
#include "tracker.h"

constexpr const char* SESSION_FILENAME = ".session";

struct Torrent;
struct TorrentInfo;
struct InternetInfo;
// struct Tracker;

class SessionManager : public QObject
{
    Q_OBJECT

    std::unique_ptr<lt::session> m_session;
    // QHash<std::uint32_t, lt::torrent_handle> m_torrentHandles;
    QHash<std::uint32_t, TorrentHandle> m_torrentHandles;

    QTimer m_alertTimer;
    QTimer m_resumeDataTimer;

    std::int64_t m_currentTorrentId{-1};

    explicit SessionManager(QObject *parent = nullptr);
public:
    Q_DISABLE_COPY_MOVE(SessionManager);
    ~SessionManager();

    static SessionManager& instance() {
        static SessionManager sessionManager{nullptr};
        return sessionManager;
    }

    bool addTorrentByFilename(QStringView filepath, QStringView outputDir);
    bool addTorrentByMagnet(QString magnetURI, QStringView outputDir);

    bool isTorrentPaused(const std::uint32_t) const;
    void pauseTorrent(const std::uint32_t id);
    void resumeTorrent(const std::uint32_t id);
    void removeTorrent(const std::uint32_t id, bool removeWithContents);

    void loadResumes();

    // Managing session
    void setDownloadLimit(int value);
    void setUploadLimit(int value);


    // Peer
    void setCurrentTorrentId(std::int64_t value) {
        m_currentTorrentId = value;
    }
private:
    lt::session_params loadSessionParams();

    // Event loop and alert functions
    void eventLoop();
    void updateProperties();
    void updateGeneralProperty(const lt::torrent_handle& handle);
    void handleFinishedAlert(lt::torrent_finished_alert* alert);
    void handleStateUpdateAlert(lt::state_update_alert* alert);
    void handleMetadataReceived(lt::metadata_received_alert* alert);
    void handleResumeDataAlert(lt::save_resume_data_alert* alert);
    void handleAddTorrentAlert(lt::add_torrent_alert* alert);

    void saveResumes();
    bool addTorrent(lt::add_torrent_params params);
    void handleStatusUpdate(const lt::torrent_status& status, const lt::torrent_handle& handle);

    bool isTorrentExists(const lt::sha1_hash& hash) const;
signals:
    void torrentAdded(const Torrent& torrent);
    void torrentUpdated(const Torrent& torrent);
    void torrentFinished(const std::uint32_t id, const lt::torrent_status& status);
    void torrentDeleted(const std::uint32_t id);

    void peerInfo(const std::uint32_t id, const std::vector<lt::peer_info>& peers);
    void clearPeerInfo();

    void generalInfo(const TorrentInfo& tInfo, const InternetInfo& iInfo);
    void clearGeneralInfo();

    void trackersInfo(const QList<Tracker>& trackers);
    void clearTrackers();

    void urlSeedsInfo(const std::set<std::string>&);
    void clearUrlSeeds();

    void pieceBarInfo(const lt::typed_bitfield<lt::piece_index_t>& pieces, const std::vector<int>& downloadingPiecesIdx);
};

inline std::vector<char> readFile(const char *filename)
{
    QFile file{filename};
    if (file.open(QIODevice::ReadOnly)) {
        auto bytes = file.readAll();
        return std::vector<char>{bytes.begin(), bytes.end()};
    }
    return {};
}

inline void writeTorrentFile(std::shared_ptr<const lt::torrent_info> ti) {
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + "torrents";

    auto stateFilePath = stateDirPath + QDir::separator() + QString{lt::aux::to_hex(ti->info_hashes().get_best().to_string()).c_str()} + ".torrent";
    QFile file{stateFilePath};

    if (file.open(QIODevice::WriteOnly)) {
        lt::create_torrent ci{*ti};
        auto buf = ci.generate_buf();
        file.write(buf.data(), buf.size());

        file.flush();
        file.close();
    }
}

inline void saveResumeData(std::shared_ptr<const lt::torrent_info> ti, const std::vector<char>& buf) {
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + "state";
    auto stateFilePath = stateDirPath + QDir::separator() + QString{lt::aux::to_hex(ti->info_hashes().get_best().to_string()).c_str()} + ".fastresume";
    QFile file{stateFilePath};
    if (file.open(QIODevice::WriteOnly)) {
        file.write(buf.data(), buf.size());

        file.flush();
        file.close();
    }
}

#endif // SESSIONMANAGER_H
