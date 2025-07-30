#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <fstream>
#include <vector>
#include <libtorrent/libtorrent.hpp>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>

struct Torrent;
constexpr const char* SESSION_FILENAME = ".session";

class SessionManager : public QObject
{
    Q_OBJECT

    std::unique_ptr<lt::session> m_session;
    // QList<lt::torrent_handle> m_torrentHandles;
    QHash<std::uint32_t, lt::torrent_handle> m_torrentHandles;

    QTimer m_alertTimer;
    QTimer m_resumeDataTimer;
public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager();

    void addTorrentByFilename(QStringView filepath);
    void addTorrentByMagnet(QString magnetURI);

    bool isTorrentPaused(const std::uint32_t) const;
    void pauseTorrent(const std::uint32_t id);
    void resumeTorrent(const std::uint32_t id);
    void removeTorrent(const std::uint32_t id, bool removeWithContents);

    void loadResumes();
private:
    lt::session_params loadSessionParams();

    // Event loop and alert functions
    void eventLoop();
    void handleFinishedAlert(lt::torrent_finished_alert* alert);
    void handleStateUpdateAlert(lt::state_update_alert* alert);
    void handleMetadataReceived(lt::metadata_received_alert* alert);
    void handleResumeDataAlert(lt::save_resume_data_alert* alert);

    void saveResumes();
    void addTorrent(lt::add_torrent_params params);
    void handleStatusUpdate(const lt::torrent_status& status, const lt::torrent_handle& handle);
signals:
    void torrentAdded(const Torrent& torrent);
    void torrentUpdated(const Torrent& torrent);
    void torrentFinished(const std::uint32_t id, const lt::torrent_status& status);
    void torrentDeleted(const std::uint32_t id);
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
