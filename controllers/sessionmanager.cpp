#include "sessionmanager.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include "torrent.h"
#include <libtorrent/libtorrent.hpp>
#include <QSettings>
#include "settingsvalues.h"
#include "torrentinfo.h"
#include <QDateTime>
#include "tracker.h"

SessionManager::SessionManager(QObject *parent)
    : QObject{parent}
{
    auto sessParams = loadSessionParams();
    m_session = std::make_unique<lt::session>(std::move(sessParams));

    connect(&m_alertTimer, &QTimer::timeout, this, &SessionManager::eventLoop);
    m_alertTimer.start(1000);
    connect(&m_resumeDataTimer, &QTimer::timeout, this, &SessionManager::saveResumes);
    m_resumeDataTimer.start(2000); // Check if torrent handles need save_resume, and then save .fastresume
}

SessionManager::~SessionManager()
{
    auto sessionFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + SESSION_FILENAME;
    QFile file{sessionFilePath};
    if (file.open(QIODevice::WriteOnly)) {
        auto sessionData = lt::write_session_params_buf(m_session->session_state());
        if (!sessionData.empty()) {
            file.write(sessionData.data(), sessionData.size());
        }
        file.flush();
        file.close();
    }
}

libtorrent::session_params SessionManager::loadSessionParams()
{
    auto sessionFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + SESSION_FILENAME;
    auto sessionFileContents = readFile(sessionFilePath.toUtf8().constData());
    lt::session_params sessParams;
    if (sessionFileContents.empty()) {
        sessParams.settings.set_int(
            lt::settings_pack::alert_mask,
            lt::alert_category::status |
            lt::alert_category::error |
            lt::alert_category::storage
        );

        QSettings settings;
        int downloadSpeedLimit = settings.value(SettingsValues::SESSION_DOWNLOAD_SPEED_LIMIT, QVariant{0}).toInt();
        int uploadSpeedLimit = settings.value(SettingsValues::SESSION_UPLOAD_SPEED_LIMIT, QVariant{0}).toInt();
        sessParams.settings.set_int(lt::settings_pack::download_rate_limit, downloadSpeedLimit);
        sessParams.settings.set_int(lt::settings_pack::upload_rate_limit, uploadSpeedLimit);
    } else {
        sessParams = std::move(lt::read_session_params(sessionFileContents));
    }
    return sessParams;
}


void SessionManager::saveResumes()
{
    for (auto& torrent : m_torrentHandles) {
        if (torrent.isNeedSaveData() && torrent.isValid()) {
            // torrent.save_resume_data();
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
        }
        if (auto* statusAlert = lt::alert_cast<lt::state_update_alert>(alert)) {
            handleStateUpdateAlert(statusAlert);
        }
        if (auto* metadataReceivedAlert = lt::alert_cast<lt::metadata_received_alert>(alert)) {
            handleMetadataReceived(metadataReceivedAlert);
        }
        if (auto* resumeDataAlert = lt::alert_cast<lt::save_resume_data_alert>(alert)) {
            handleResumeDataAlert(resumeDataAlert);
        }
        if (auto* addTorrentAlert = lt::alert_cast<lt::add_torrent_alert>(alert)) {
            handleAddTorrentAlert(addTorrentAlert);
        }
        if (auto* stateAlert = lt::alert_cast<lt::session_stats_alert>(alert)) { // if happens every second so i think i wont use chrono stuff
            // TODO: Factor out in a function
            const auto& counters = stateAlert->counters();
            auto recvPayloadBytes = counters[lt::counters::recv_bytes];
            auto newRecv = recvPayloadBytes - lastSessionRecvPayloadBytes;
            lastSessionRecvPayloadBytes = recvPayloadBytes;

            auto uploadPayloadBytes = counters[lt::counters::sent_bytes];
            auto newUpload = uploadPayloadBytes - lastSessionUploadPayloadBytes;
            lastSessionUploadPayloadBytes = uploadPayloadBytes;

            emit chartPoint(newRecv, newUpload);
        }
    }

    m_session->post_torrent_updates();
    m_session->post_session_stats();
    updateProperties();

    qDebug() << "Took" << timer.elapsed();
}

void SessionManager::updateProperties()
{
    if (m_currentTorrentId != -1) {
        auto& handle = m_torrentHandles[m_currentTorrentId];
        {
            if (handle.isValid()) {
                std::vector<lt::peer_info> peers = m_torrentHandles[m_currentTorrentId].getPeerInfo();
                emit peerInfo(m_currentTorrentId, std::move(peers));

                auto trackers = handle.getTrackers();
                QList<Tracker> tracks;
                for (const auto& tracker : trackers) {
                    Tracker tr{};
                    tr.url = QString::fromStdString(tracker.url);
                    tr.tier = tracker.tier;
                    for (const auto& ep : tracker.endpoints) {
                        if (ep.enabled) {
                            tr.isWorking = true;
                        }
                        const auto& ihash = ep.info_hashes[lt::protocol_version::V1];
                        tr.seeds = ihash.scrape_complete == -1 ? 0 : ihash.scrape_complete;
                        tr.leeches = ihash.scrape_incomplete == -1 ? 0 : ihash.scrape_incomplete;
                        tr.message = QString::fromStdString(ihash.message);
                    }

                    tracks.append(std::move(tr));
                }
                emit trackersInfo(tracks);


                auto urlSeeds = m_torrentHandles[m_currentTorrentId].handle().url_seeds();
                emit urlSeedsInfo(urlSeeds);
            } else { // if its not valid something is wrong
                m_currentTorrentId = -1;
            }
        }
    } else {
        // clear stats
        emit clearPeerInfo();
        emit clearGeneralInfo();
        emit clearTrackers();
        emit clearUrlSeeds();
    }

}

void SessionManager::handleFinishedAlert(libtorrent::torrent_finished_alert *alert)
{
    auto handles = m_torrentHandles.values();
    auto pos = std::find_if(handles.begin(), handles.end(), [alert](auto&& handle) {
        return handle.id() == alert->handle.id();
    });

    pos->saveResumeData();
    // Otherwise it's behaving kinda weird
    emit torrentFinished(alert->handle.id(), alert->handle.status());
}

void SessionManager::handleStateUpdateAlert(libtorrent::state_update_alert *alert)
{
    auto statuses = alert->status;
    for (auto& status : statuses) {
        auto& handle = status.handle;

        if (handle.id() == m_currentTorrentId) {
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
    iInfo.downloaded = status.total_download;
    iInfo.downSpeed = status.download_rate;
    iInfo.downLimit = handle.download_limit();

    iInfo.eta = status.download_rate == 0 ? -1 : (status.total_wanted - status.total_wanted_done) / status.download_rate;

    iInfo.uploaded = status.total_upload;
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
    tInfo.hashBest = QString::fromStdString(lt::aux::to_hex(handle.info_hashes().get_best().to_string()));
    tInfo.savePath = QString::fromStdString(status.save_path);
    tInfo.comment = "Something";

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
    bool IsPaused = (status.flags & (lt::torrent_flags::paused)) == lt::torrent_flags::paused ? true : false;

    Torrent torrent = {
        handle.id(),
        QString::fromStdString(status.name),
        // QString::number(status.total_wanted / 1024.0 / 1024.0) + " MB",
        static_cast<std::uint64_t>(status.total_wanted),
        std::ceil(((status.total_done / 1024.0 / 1024.0) / (status.total_wanted / 1024.0 / 1024.0) * 100.0) * 100) / 100.0,
        !IsPaused ? torrentStateToString(status.state) : "Stopped",
        status.num_seeds,
        status.num_peers,
        // QString::number(std::ceil(status.download_rate / 1024.0 / 1024.0 * 100.0) / 100.0) + " MB/s",
        static_cast<std::uint64_t>(status.download_rate), // TODO: Replace uint64_t with int, since it is used by torrent_status
        // QString::number(std::ceil(status.upload_rate / 1024.0 / 1024.0 * 100.0) / 100.0) + " MB/s",
        static_cast<std::uint64_t>(status.upload_rate),
        status.download_rate == 0 ? -1 : (status.total_wanted - status.total_wanted_done) / status.download_rate,
    };
    emit torrentUpdated(torrent);
}


void SessionManager::handleMetadataReceived(libtorrent::metadata_received_alert *alert)
{
    writeTorrentFile(alert->handle.torrent_file());
}

void SessionManager::handleResumeDataAlert(libtorrent::save_resume_data_alert *alert)
{
    if (alert->handle.is_valid() && alert->handle.torrent_file()) {
        auto resumeDataBuf = lt::write_resume_data_buf(alert->params);
        saveResumeData(alert->handle.torrent_file(), resumeDataBuf);
    }
}

void SessionManager::handleAddTorrentAlert(libtorrent::add_torrent_alert *alert)
{
    auto& torrent_handle = alert->handle;
    m_torrentHandles.insert(torrent_handle.id(), TorrentHandle{torrent_handle});

    // No point setting status fields, since they are zeroed and will be filled on status alert
    Torrent torrent = {
        torrent_handle.id(),
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

void SessionManager::loadResumes()
{
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + "state";
    QDir dir{stateDirPath};
    auto entries = dir.entryList(QDir::Filter::Files);
    for (auto& entry : entries) {
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
    auto asBytes = value * 1024; // value is in kilobytes per second
    newSettings.set_int(lt::settings_pack::download_rate_limit, asBytes);
    m_session->apply_settings(std::move(newSettings));

    QSettings settings;
    settings.setValue(SettingsValues::SESSION_DOWNLOAD_SPEED_LIMIT, QVariant{asBytes});
}

void SessionManager::setUploadLimit(int value)
{
    lt::settings_pack newSettings = m_session->get_settings();
    auto asBytes = value * 1024; // value is in kilobytes per second
    newSettings.set_int(lt::settings_pack::upload_rate_limit, asBytes);
    m_session->apply_settings(std::move(newSettings));

    QSettings settings;
    settings.setValue(SettingsValues::SESSION_UPLOAD_SPEED_LIMIT, QVariant{asBytes});
}

void SessionManager::banPeers(const QList<QPair<QString, unsigned short> > &bannablePeers)
{
    lt::ip_filter filter = m_session->get_ip_filter();
    for (const auto& peer : bannablePeers) {
        auto address = boost::asio::ip::make_address(peer.first.toUtf8().constData());
        filter.add_rule(address, address, lt::ip_filter::blocked);
    }
    m_session->set_ip_filter(filter);
}

void SessionManager::addPeerToCurrentTorrent(const boost::asio::ip::tcp::endpoint &ep)
{
    if (m_currentTorrentId == -1) {
        throw std::runtime_error("Torrent is not set");
    }
    m_torrentHandles[m_currentTorrentId].connectToPeer(ep);
}

void SessionManager::addPeersToCurrentTorrent(const QList<boost::asio::ip::tcp::endpoint> &eps)
{
    if (m_currentTorrentId == -1) {
        throw std::runtime_error("Torrent is not set");
    }
    auto& torrentHandle = m_torrentHandles[m_currentTorrentId];
    for (const auto& ep : eps) {
        torrentHandle.connectToPeer(ep);
    }
}

bool SessionManager::addTorrentByFilename(QStringView filepath, QStringView outputDir)
{
    auto torrent_info = std::make_shared<lt::torrent_info>(filepath.toUtf8().toStdString());
    lt::add_torrent_params params{};
    writeTorrentFile(torrent_info);

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

void SessionManager::removeTorrent(const uint32_t id, bool removeWithContents)
{
    if (id == m_currentTorrentId) {
        m_currentTorrentId = -1;
    }

    if (removeWithContents) {
        m_session->remove_torrent(m_torrentHandles[id].handle(), lt::session::delete_files);
    } else {
        m_session->remove_torrent(m_torrentHandles[id].handle());
    }
    auto& torrentHandle = m_torrentHandles[id];

    // Delete .fastresume and .torrent
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto torrentsPath = basePath + QDir::separator() + "torrents";

    auto hashString = torrentHandle.bestHashAsString();
    auto torrentFile = torrentsPath + QDir::separator() + hashString + ".torrent";
    if (!QFile::remove(torrentFile)) {
        qDebug() << "Could not remove .torrent file";
    }

    auto statePath = basePath + QDir::separator() + "state";
    auto stateFile = statePath + QDir::separator() + hashString + ".fastresume";
    if (!QFile::remove(stateFile)) {
        qDebug() << "Could not remove .fastresume file";
    }

    m_torrentHandles.remove(id);
    emit torrentDeleted(id);
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
