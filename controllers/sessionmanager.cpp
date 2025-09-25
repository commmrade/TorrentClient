#include "sessionmanager.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include "torrent.h"
#include <libtorrent/libtorrent.hpp>
#include <QSettings>
#include "settingsvalues.h"
#include "torrentinfo.h"
#include "tracker.h"
#include "utils.h"
#include "category.h"
#include "dirs.h"




SessionManager::SessionManager(QObject *parent)
    : QObject{parent}
{
    auto sessParams = loadSessionParams();
    m_session = std::make_unique<lt::session>(std::move(sessParams));

    connect(&m_alertTimer, &QTimer::timeout, this, &SessionManager::eventLoop);
    m_alertTimer.start(1000); // !!!!: Dont change, breaks graphs and other stuff that require something per second
    connect(&m_resumeDataTimer, &QTimer::timeout, this, &SessionManager::saveResumes);
    m_resumeDataTimer.start(2000); // Check if torrent handles need save_resume, and then save .fastresume
}

SessionManager::~SessionManager()
{
    auto sessionFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + SESSION_FILENAME;

    QFile file{sessionFilePath};
    if (file.open(QIODevice::WriteOnly)) {
        auto sessionData = lt::write_session_params_buf(m_session->session_state());
        file.write(sessionData.data(), sessionData.size());
        file.flush();
        file.close();
    }
}

libtorrent::session_params SessionManager::loadSessionParams()
{
    auto sessionFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + SESSION_FILENAME;
    auto sessionFileContents = detail::readFile(sessionFilePath.toUtf8().constData());
    lt::session_params sessParams;
    if (sessionFileContents.empty()) {
        sessParams.settings.set_int(
            lt::settings_pack::alert_mask,
            lt::alert_category::status |
            lt::alert_category::error |
            lt::alert_category::storage
        );

        loadSessionSettingsFromSettings(sessParams);
    } else {
        sessParams = std::move(lt::read_session_params(sessionFileContents));
    }
    return sessParams;
}

void SessionManager::loadSessionSettingsFromSettings(lt::session_params& sessParams)
{
    QSettings settings;
    int downloadSpeedLimit = settings.value(SettingsValues::SESSION_DOWNLOAD_SPEED_LIMIT, QVariant{0}).toInt();
    int uploadSpeedLimit = settings.value(SettingsValues::SESSION_UPLOAD_SPEED_LIMIT, QVariant{0}).toInt();
    sessParams.settings.set_int(lt::settings_pack::download_rate_limit, downloadSpeedLimit);
    sessParams.settings.set_int(lt::settings_pack::upload_rate_limit, uploadSpeedLimit);
}

void SessionManager::saveResumes()
{
    for (auto& torrent : m_torrentHandles) {
        if (torrent.isValid() && torrent.isNeedSaveData()) {
            torrent.saveResumeData();
        }
    }
}

void SessionManager::eventLoop()
{
    QElapsedTimer timer;
    timer.start();

    std::vector<lt::alert*> alerts;
    m_session->pop_alerts(&alerts);
    for (auto* alert : alerts) {
        if (auto* finished_alert = lt::alert_cast<lt::torrent_finished_alert>(alert)) {
            handleFinishedAlert(finished_alert);
        } else if (auto* statusAlert = lt::alert_cast<lt::state_update_alert>(alert)) {
            handleStateUpdateAlert(statusAlert);
        } else if (auto* metadataReceivedAlert = lt::alert_cast<lt::metadata_received_alert>(alert)) {
            handleMetadataReceived(metadataReceivedAlert);
        } else if (auto* resumeDataAlert = lt::alert_cast<lt::save_resume_data_alert>(alert)) {
            handleResumeDataAlert(resumeDataAlert);
        } else if (auto* addTorrentAlert = lt::alert_cast<lt::add_torrent_alert>(alert)) {
            handleAddTorrentAlert(addTorrentAlert);
        } else if (auto* stateAlert = lt::alert_cast<lt::session_stats_alert>(alert)) { // if happens every second so i think i wont use chrono stuff
            handleSessionStatsAlert(stateAlert);
        } else if (auto* torrentErrorAlert = lt::alert_cast<lt::torrent_error_alert>(alert)) {
            handleTorrentErrorAlert(torrentErrorAlert);
        }
    }

    m_session->post_torrent_updates();
    m_session->post_session_stats(); // Needed for graphs
    updateProperties();

    // qDebug() << "Loop elapsed:" << timer.elapsed() << "Msecs";
}

// HERE: Separate function for each emit thingy
void SessionManager::updateProperties()
{
    if (m_currentTorrentId.has_value()) {
        auto currentTorrentId = m_currentTorrentId.value();
        auto& handle = m_torrentHandles[currentTorrentId];
        {
            if (handle.isValid()) {
                // Update peers
                updatePeersProp(handle);

                // Update trackers
                updateTrackersProp(handle);

                // Update files
                updateFilesProp(handle);

                // update url seeds
                updateUrlProp(handle);
            } else { // if its not valid something is wrong
                m_currentTorrentId = std::nullopt;
            }
        }
    } else {
        // clear stats
        emitClearSignals();
    }

}

void SessionManager::updatePeersProp(TorrentHandle& handle)
{
    std::vector<lt::peer_info> peers = handle.getPeerInfo();
    emit peerInfo(handle.id(), std::move(peers));
}

void SessionManager::updateTrackersProp(TorrentHandle &handle)
{
    auto ltTrackers = handle.getTrackers();
    QList<Tracker> trackers;

    const bool hasV2 = handle.handle().info_hashes().has_v2();
    const auto version = hasV2 ? lt::protocol_version::V2 : lt::protocol_version::V1;

    for (const auto& tracker : ltTrackers) {
        Tracker tr{};
        tr.url = QString::fromStdString(tracker.url);
        tr.tier = tracker.tier;
        for (const auto& ep : tracker.endpoints) {
            if (ep.enabled) {
                tr.isWorking = true;
            }
            const lt::announce_infohash ihash = ep.info_hashes[version];
            tr.seeds = ihash.scrape_complete == -1 ? 0 : ihash.scrape_complete;
            tr.leeches = ihash.scrape_incomplete == -1 ? 0 : ihash.scrape_incomplete;
            tr.message = QString::fromStdString(ihash.message);
        }

        trackers.append(std::move(tr));
    }
    emit trackersInfo(trackers);
}

void SessionManager::updateFilesProp(TorrentHandle &handle)
{
    auto fileListFromTorrentInfo = [this](const TorrentHandle& handle, std::shared_ptr<const lt::torrent_info> tinfo) -> QList<File> {
        const auto& files = tinfo->files();
        auto num_files = files.num_files();

        QList<File> result;
        auto fileProgresses = handle.handle().file_progress();
        for (auto i = 0; i < num_files; ++i) {
            auto fileSize = files.file_size(i);
            File file;
            file.id = i;
            file.isEnabled = handle.handle().file_priority(i) != lt::dont_download;
            file.filename = QString::fromStdString(files.file_path(i));
            file.filesize = files.file_size(i);
            file.downloaded = fileProgresses[i];
            file.priority = handle.handle().file_priority(i);
            result.append(std::move(file));
        }
        return result;
    };

    if (auto tinfo = handle.handle().torrent_file()) {
        auto files = fileListFromTorrentInfo(handle, tinfo);
        emit filesInfo(files);
    }
}

void SessionManager::updateUrlProp(TorrentHandle &handle)
{
    auto urlSeeds = handle.handle().url_seeds();
    emit urlSeedsInfo(urlSeeds);
}

void SessionManager::updateTorrent(TorrentHandle &torrentHandle, const libtorrent::torrent_status &status)
{
    // TODO: FIlter information in status, torrentHandle.status() is expensive, so I can get rid of expensive information there if not needed
    bool isPaused = torrentHandle.isPaused();
    double progress = std::ceil((static_cast<double>(status.total_wanted_done) / static_cast<double>(status.total_wanted) * 100.0) * 100) / 100.0;

    qDebug() << "Update torrent";
    torrentHandle.resetCategory(); // sync category justin case
    Torrent torrent = {
        torrentHandle.id(),
        torrentHandle.getCategory(), // Default category is All, TODO: This may fuck up category changing, in torrent table model i check if category is empty leave the current category
        QString::fromStdString(status.name),
        // QString::number(status.total_wanted / 1024.0 / 1024.0) + " MB",
        status.total_wanted,
        progress,
        !isPaused ? torrentStateToString(status.state) : "Stopped",
        status.num_seeds,
        status.num_peers,
        // QString::number(std::ceil(status.download_rate / 1024.0 / 1024.0 * 100.0) / 100.0) + " MB/s",
        status.download_payload_rate, // count only pieces, without protocol stuff
        // status.download_rate,
        // QString::number(std::ceil(status.upload_rate / 1024.0 / 1024.0 * 100.0) / 100.0) + " MB/s",
        status.upload_rate,
        status.download_rate == 0 ? -1 : (status.total_wanted - status.total_wanted_done) / status.download_rate,
    };
    emit torrentUpdated(torrent);
}

void SessionManager::handleFinishedAlert(libtorrent::torrent_finished_alert *alert)
{
    auto handles = m_torrentHandles.values();
    auto pos = std::find_if(handles.begin(), handles.end(), [alert](auto&& handle) {
        return handle.id() == alert->handle.id();
    });

    pos->saveResumeData(); // Without it it does weird stuff
    pos->setCategory(Categories::SEEDING);
    emit torrentFinished(alert->handle.id(), alert->handle.status());
}

void SessionManager::handleStateUpdateAlert(libtorrent::state_update_alert *alert)
{
    auto statuses = alert->status;
    int currentId = m_currentTorrentId.has_value() ? m_currentTorrentId.value() : -1;
    for (auto& status : statuses) {
        auto& handle = status.handle;
        auto& h = m_torrentHandles[handle.id()];

        if (handle.id() == currentId) {
            updateGeneralProperty(handle);
        }
        handleStatusUpdate(status, handle);
    }
}

void SessionManager::updateGeneralProperty(const lt::torrent_handle& handle)
{
    auto status = handle.status();
    InternetInfo iInfo;
    iInfo.activeTime = status.active_duration.count();
    iInfo.downloaded = status.all_time_download; // dependent on .fastresum
    iInfo.downSpeed = status.download_rate;
    iInfo.downLimit = handle.download_limit();

    iInfo.eta = status.download_rate == 0 ? -1 : (status.total_wanted - status.total_wanted_done) / status.download_rate;

    iInfo.uploaded = status.all_time_upload; // dependent on .fastresum
    iInfo.upSpeed = status.upload_rate;
    iInfo.upLimit = handle.upload_limit();

    iInfo.connections = status.num_connections;
    iInfo.seeds = status.num_seeds;
    iInfo.peers = status.num_peers;

    TorrentInfo tInfo;
    // qDebug() << "Compl time:" << status.completed_time;
    tInfo.completedTime = status.completed_time;
    tInfo.size = status.total_wanted;
    tInfo.startTime = status.added_time;

    // tInfo.hashBest = utils::toHex(handle.info_hashes().get_best().to_string());
    tInfo.hashV1 = utils::toHex(handle.info_hashes().v1.to_string());

    auto hashV2 = handle.info_hashes().v2;
    tInfo.hashV2 = utils::toHex(hashV2.is_all_zeros() ? "" : hashV2.to_string());

    tInfo.savePath = QString::fromStdString(status.save_path);

    {
        auto t_file = handle.torrent_file();
        tInfo.comment = t_file ? QString::fromStdString(t_file->comment()) : "";
    }

    tInfo.piecesCount = status.pieces.size();

    auto tfile = handle.torrent_file();
    if (tfile) {
        tInfo.pieceSize = tfile->piece_length();
    }

    emit generalInfo(tInfo, iInfo);

    // Update pieces bar
    std::vector<int> downloadingPiecesIndices;

    std::vector<lt::partial_piece_info> pieces;
    handle.get_download_queue(pieces);

    for (const auto& pcs : pieces) {
        if (pcs.finished && pcs.finished < pcs.blocks_in_piece) { // finished more than 1, finished == blocks means finished
            downloadingPiecesIndices.push_back(pcs.piece_index);
        }
    }

    emit pieceBarInfo(status.pieces, downloadingPiecesIndices);
}

void SessionManager::handleStatusUpdate(const lt::torrent_status& status, const libtorrent::torrent_handle &handle)
{
    auto& torrentHandle = m_torrentHandles[handle.id()];
    if (!torrentHandle.isValid()) return;
    // bool IsPaused = (status.flags & (lt::torrent_flags::paused)) == lt::torrent_flags::paused ? true : false;
    // updateTorrent(torrentHandle, status);

}

void SessionManager::handleMetadataReceived(libtorrent::metadata_received_alert *alert)
{
    detail::writeTorrentFile(alert->handle.torrent_file());
}

void SessionManager::handleResumeDataAlert(libtorrent::save_resume_data_alert *alert)
{
    if (alert->handle.is_valid() && alert->handle.torrent_file()) {
        auto resumeDataBuf = lt::write_resume_data_buf(alert->params);
        detail::saveResumeData(alert->handle.torrent_file(), resumeDataBuf);
    }
}

void SessionManager::handleAddTorrentAlert(libtorrent::add_torrent_alert *alert)
{
    auto& torrent_handle = alert->handle;
    m_torrentHandles.insert(torrent_handle.id(), TorrentHandle{torrent_handle});
    m_torrentHandles[torrent_handle.id()].setCategory(Categories::RUNNING); // this is gonna be handy if im ever gonna let user add torrents as paused

    // No point setting status fields, since they are zeroed and will be filled on status alert
    Torrent torrent = {
        torrent_handle.id(),
        QString{},
        QString::fromStdString(torrent_handle.status().name),
        0,
        0.0,
        torrentStateToString(torrent_handle.status().state),
        0,
        0,
        0,
        0
    };

    emit torrentAdded(torrent);
}

void SessionManager::handleSessionStatsAlert(libtorrent::session_stats_alert *alert)
{
    const auto& counters = alert->counters();
    auto recvPayloadBytes = counters[lt::counters::recv_payload_bytes];
    auto newRecv = recvPayloadBytes - lastSessionRecvPayloadBytes;
    lastSessionRecvPayloadBytes = recvPayloadBytes;

    auto uploadPayloadBytes = counters[lt::counters::sent_bytes];
    auto newUpload = uploadPayloadBytes - lastSessionUploadPayloadBytes;
    lastSessionUploadPayloadBytes = uploadPayloadBytes;

    emit chartPoint(newRecv, newUpload);
}

void SessionManager::handleTorrentErrorAlert(libtorrent::torrent_error_alert *alert)
{
    qDebug() << "Torrent Error Alert:" << alert->message();
    auto id = alert->handle.id();
    auto& torrentHandle = m_torrentHandles[id];
    torrentHandle.setCategory(Categories::FAILED);
}

void SessionManager::loadResumes()
{
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + Dirs::STATE;
    QDir dir{stateDirPath};
    auto entries = dir.entryList(QDir::Filter::Files);
    for (auto& entry : entries) {
        if (!entry.endsWith(".fastresume")) {
            qWarning() << "Wrong formatted file in state directory";
            continue;
        }

        QFile file{stateDirPath + QDir::separator() + entry};
        if (file.open(QIODevice::ReadOnly)) {
            auto buffer = file.readAll();
            auto params = lt::read_resume_data(buffer);
            bool isPaused = (params.flags & (lt::torrent_flags::paused)) == lt::torrent_flags::paused ? true : false;
            if (isPaused) {
                params.flags &= ~lt::torrent_flags::auto_managed;
            }
            m_session->async_add_torrent(std::move(params));
        }
    }
}

void SessionManager::setDownloadLimit(int value)
{
    lt::settings_pack newSettings = m_session->get_settings();
    newSettings.set_int(lt::settings_pack::download_rate_limit, value);
    m_session->apply_settings(std::move(newSettings));

    QSettings settings;
    settings.setValue(SettingsValues::SESSION_DOWNLOAD_SPEED_LIMIT, QVariant{value});
}

void SessionManager::setUploadLimit(int value)
{
    lt::settings_pack newSettings = m_session->get_settings();
    newSettings.set_int(lt::settings_pack::upload_rate_limit, value);
    m_session->apply_settings(std::move(newSettings));

    QSettings settings;
    settings.setValue(SettingsValues::SESSION_UPLOAD_SPEED_LIMIT, QVariant{value});
}

void SessionManager::changeFilePriority(std::uint32_t id, int fileIndex, int priority)
{
    auto& handle = m_torrentHandles[id];
    // No need to unset auto_managed, since it onyl takes care of pausing/unpausing and some other stuff, but definitely not files' priorities
    handle.setFilePriority(fileIndex, priority);
}

void SessionManager::renameFile(uint32_t id, int fileIndex, const QString& newName)
{
    auto& handle = m_torrentHandles[id];
    handle.renameFile(fileIndex, newName);
}

void SessionManager::banPeers(const QList<QPair<QString, unsigned short>> &bannablePeers)
{
    lt::ip_filter filter = m_session->get_ip_filter();
    for (const auto& peer : bannablePeers) {
        auto address = boost::asio::ip::make_address(peer.first.toUtf8().constData());
        filter.add_rule(address, address, lt::ip_filter::blocked);
    }
    m_session->set_ip_filter(filter);
}

void SessionManager::addPeerToTorrent(std::uint32_t id, const boost::asio::ip::tcp::endpoint &ep)
{
    assert(id < m_torrentHandles.size());
    m_torrentHandles[id].connectToPeer(ep);
}

void SessionManager::addPeersToTorrent(std::uint32_t id, const QList<boost::asio::ip::tcp::endpoint> &eps)
{
    assert(id < m_torrentHandles.size());
    auto& torrentHandle = m_torrentHandles[id];
    for (const auto& ep : eps) {
        torrentHandle.connectToPeer(ep);
    }
}

void SessionManager::setCurrentTorrentId(std::optional<uint32_t> value)
{
    if (m_currentTorrentId.has_value() && m_currentTorrentId.value() == value) {
        return;
    }
    m_currentTorrentId = value;
}

void SessionManager::forceUpdateProperties() {
    auto& torrentHandle = m_torrentHandles[m_currentTorrentId.value()];
    updatePeersProp(torrentHandle);
    updateGeneralProperty(torrentHandle.handle());
    updateTrackersProp(torrentHandle);
    updateUrlProp(torrentHandle);
    updateFilesProp(torrentHandle);
}

void SessionManager::forceUpdateCategory()
{
    // TODO: Upate category and signal so torrent list immediately changes after selecting different category
}

void SessionManager::emitClearSignals()
{
    emit clearPeerInfo();
    emit clearGeneralInfo();
    emit clearTrackers();
    emit clearUrlSeeds();
    emit clearFiles();
}

bool SessionManager::addTorrentByFilename(QStringView filepath, QStringView outputDir)
{
    auto torrent_info = std::make_shared<lt::torrent_info>(filepath.toUtf8().toStdString());
    lt::add_torrent_params params{};
    detail::writeTorrentFile(torrent_info);

    params.ti = std::move(torrent_info);
    params.save_path = outputDir.toString().toStdString();
    return addTorrent(params);
}

bool SessionManager::addTorrentByMagnet(QString magnetURI, QStringView outputDir)
{
    auto params = lt::parse_magnet_uri(magnetURI.toStdString());
    // qDebug() << "magnet torrent" << params.name;
    params.save_path = outputDir.toString().toStdString();
    return addTorrent(std::move(params));
}

bool SessionManager::addTorrentByTorrentInfo(std::shared_ptr<const libtorrent::torrent_info> ti, QStringView outputDir)
{
    lt::add_torrent_params params{};
    detail::writeTorrentFile(ti);

    params.ti = std::make_shared<lt::torrent_info>(*ti);
    params.save_path = outputDir.toString().toStdString();
    return addTorrent(params);
}

bool SessionManager::isTorrentPaused(const uint32_t id) const
{
    auto& torrentHandle = m_torrentHandles[id];
    return torrentHandle.isPaused();
}

void SessionManager::pauseTorrent(const uint32_t id)
{
    m_torrentHandles[id].pause();
}

void SessionManager::resumeTorrent(const uint32_t id)
{
    m_torrentHandles[id].resume();
}

bool SessionManager::removeTorrent(const uint32_t id, bool removeWithContents)
{
    if (m_currentTorrentId.has_value() && id == m_currentTorrentId.value()) {
        qWarning() << "Current torrent set to null";
        m_currentTorrentId = std::nullopt;
    }

    if (removeWithContents) {
        m_session->remove_torrent(m_torrentHandles[id].handle(), lt::session::delete_files);
    } else {
        m_session->remove_torrent(m_torrentHandles[id].handle());
    }
    auto& torrentHandle = m_torrentHandles[id];

    // Delete .fastresume and .torrent
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto torrentsPath = basePath + QDir::separator() + Dirs::TORRENTS;

    auto hashString = torrentHandle.bestHashAsString();
    auto torrentFile = torrentsPath + QDir::separator() + hashString + FileEnding::DOT_TORRENT;

    if (!QFile::remove(torrentFile)) {
        qWarning() << "Could not remove .torrent file";
        return false;
    }

    auto statePath = basePath + QDir::separator() + Dirs::STATE;
    auto stateFile = statePath + QDir::separator() + hashString + FileEnding::DOT_FASTRESUME;
    if (!QFile::remove(stateFile)) {
        qWarning() << "Could not remove .fastresume file";
        return false;
    }

    m_torrentHandles.remove(id);
    emit torrentDeleted(id);
    return true;
}

bool SessionManager::addTorrent(libtorrent::add_torrent_params params)
{
    if (isTorrentExists(params.info_hashes.get_best().is_all_zeros() ? params.ti->info_hashes().get_best() : params.info_hashes.get_best())) {
        return false;
    }
    m_session->async_add_torrent(std::move(params));
    return true;
}


bool SessionManager::isTorrentExists(const lt::sha1_hash& hash) const
{
    auto handles = m_torrentHandles.values();
    auto torrentIter = std::find_if(handles.begin(), handles.end(), [&](const TorrentHandle& value) {
        return value.bestHash() == hash;
    });
    return torrentIter != handles.end();
}


namespace detail {
void writeTorrentFile(std::shared_ptr<const libtorrent::torrent_info> ti) {
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + Dirs::TORRENTS;

    auto torrentFilePath = stateDirPath + QDir::separator() + utils::toHex(ti->info_hashes().get_best().to_string()) + FileEnding::DOT_TORRENT;
    QFile file{torrentFilePath};

    if (file.open(QIODevice::WriteOnly)) {
        lt::create_torrent ci{*ti};
        auto buf = ci.generate_buf();
        file.write(buf.data(), buf.size());

        file.flush();
        file.close();
    }
}

void saveResumeData(std::shared_ptr<const libtorrent::torrent_info> ti, const std::vector<char> &buf) {
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + Dirs::STATE;
    auto stateFilePath = stateDirPath + QDir::separator() + utils::toHex(ti->info_hashes().get_best().to_string()) + FileEnding::DOT_FASTRESUME;
    QFile file{stateFilePath};
    if (file.open(QIODevice::WriteOnly)) {
        file.write(buf.data(), buf.size());

        file.flush();
        file.close();
    }
}

std::vector<char> readFile(const char *filename)
{
    QFile file{filename};
    if (file.open(QIODevice::ReadOnly)) {
        auto bytes = file.readAll();
        return std::vector<char>{bytes.begin(), bytes.end()};
    }
    return {};
}

} // namespace detail

