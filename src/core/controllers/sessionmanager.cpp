#include "core/controllers/sessionmanager.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include "core/utils/torrent.h"
#include <libtorrent/libtorrent.hpp>
#include <QSettings>
#include "core/utils/settingsvalues.h"
#include "core/utils/torrentinfo.h"
#include "core/utils/tracker.h"
#include "core/utils/utils.h"
#include "core/utils/category.h"
#include "core/utils/dirs.h"

SessionManager::SessionManager(QObject *parent) : QObject{parent}
{
    auto sessParams = loadSessionParams();
    m_session       = std::make_unique<lt::session>(std::move(sessParams));


    connect(&m_alertTimer, &QTimer::timeout, this, &SessionManager::eventLoop);
    QSettings settings;
    int loopDur = settings.value(SettingsNames::ADVANCED_LOOP_DURATION, SettingsValues::ADVANCED_LOOP_DURATION).toInt();
    m_alertTimer.start(loopDur);

    connect(&m_resumeDataTimer, &QTimer::timeout, this, &SessionManager::saveResumes);
    m_resumeDataTimer.start(
        2000); // Check if torrent handles need save_resume, and then save .fastresume
}

SessionManager::~SessionManager()
{
    auto sessionFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                           QDir::separator() + SESSION_FILENAME;
    if (shouldResetParams)
    {
        if (!QFile::remove(sessionFilePath))
        {
            qWarning() << "Failed to delete .session";
        }
    }
    else
    {
        QFile file{sessionFilePath};
        if (file.open(QIODevice::WriteOnly))
        {
            auto sessionData = lt::write_session_params_buf(m_session->session_state());
            file.write(sessionData.data(), sessionData.size());
            file.flush();
            file.close();
        }
    }
}

const TorrentHandle SessionManager::getTorrentHandle(const std::uint32_t id) const
{
    return m_torrentHandles[id];
}

libtorrent::session_params SessionManager::loadSessionParams()
{
    auto sessionFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                           QDir::separator() + SESSION_FILENAME;
    auto               sessionFileContents = detail::readFile(sessionFilePath.toUtf8().constData());
    lt::session_params sessParams;
    if (sessionFileContents.empty())
    {
        sessParams.settings.set_int(lt::settings_pack::alert_mask, lt::alert_category::status |
                                                                       lt::alert_category::error |
                                                                       lt::alert_category::storage);

        loadSessionSettingsFromSettings(sessParams);
    }
    else
    {
        sessParams = lt::read_session_params(sessionFileContents);
    }
    return sessParams;
}

void SessionManager::loadSessionSettingsFromSettings(lt::session_params &sessParams)
{
    QSettings settings;
    int       downloadSpeedLimit = settings
                                 .value(SettingsNames::SESSION_DOWNLOAD_SPEED_LIMIT,
                                        SettingsValues::SESSION_DOWNLOAD_SPEED_LIMIT)
                                 .toInt();
    int uploadSpeedLimit = settings
                               .value(SettingsNames::SESSION_UPLOAD_SPEED_LIMIT,
                                      SettingsValues::SESSION_UPLOAD_SPEED_LIMIT)
                               .toInt();
    sessParams.settings.set_int(lt::settings_pack::download_rate_limit, downloadSpeedLimit);
    sessParams.settings.set_int(lt::settings_pack::upload_rate_limit, uploadSpeedLimit);

    unsigned int port =
        settings.value(SettingsNames::LISTENING_PORT, SettingsValues::LISTENING_PORT_DEFAULT)
            .toUInt();
    QString listenInterfaces = QString{"0.0.0.0:%1,[::]:%1"}.arg(port);
    sessParams.settings.set_str(lt::settings_pack::listen_interfaces,
                                listenInterfaces.toStdString());

    int protocolType = settings
                           .value(SettingsNames::LISTENING_PROTOCOL,
                                  SettingsValues::LISTENING_PROTOCOL_TCP_AND_UTP)
                           .toInt();
    switch (protocolType)
    {
    case SettingsValues::LISTENING_PROTOCOL_TCP_AND_UTP:
    {
        sessParams.settings.set_bool(lt::settings_pack::enable_outgoing_tcp, true);
        sessParams.settings.set_bool(lt::settings_pack::enable_incoming_tcp, true);
        sessParams.settings.set_bool(lt::settings_pack::enable_outgoing_utp, true);
        sessParams.settings.set_bool(lt::settings_pack::enable_incoming_utp, true);
        break;
    }
    case SettingsValues::LISTENING_PROTOCOL_TCP:
    {
        sessParams.settings.set_bool(lt::settings_pack::enable_outgoing_tcp, true);
        sessParams.settings.set_bool(lt::settings_pack::enable_incoming_tcp, true);
        sessParams.settings.set_bool(lt::settings_pack::enable_outgoing_utp, false);
        sessParams.settings.set_bool(lt::settings_pack::enable_incoming_utp, false);
        break;
    }
    case SettingsValues::LISTENING_PROTOCOL_UTP:
    {
        sessParams.settings.set_bool(lt::settings_pack::enable_outgoing_tcp, false);
        sessParams.settings.set_bool(lt::settings_pack::enable_incoming_tcp, false);
        sessParams.settings.set_bool(lt::settings_pack::enable_outgoing_utp, true);
        sessParams.settings.set_bool(lt::settings_pack::enable_incoming_utp, true);
        break;
    }
    }

    int connLimit = settings
                        .value(SettingsNames::LIMITS_MAX_NUM_OF_CONNECTIONS,
                               SettingsValues::LIMITS_MAX_NUM_OF_CONNECTIONS_DEFAULT)
                        .toInt();
    sessParams.settings.set_int(lt::settings_pack::connections_limit, connLimit);

    bool dhtEnabled =
        settings
            .value(SettingsNames::PRIVACY_DHT_ENABLED, SettingsValues::PRIVACY_DHT_ENABLED_DEFAULT)
            .toBool();
    sessParams.settings.set_bool(lt::settings_pack::enable_dht, dhtEnabled);

    bool lsdEnabled = settings
                          .value(SettingsNames::PRIVACY_LOCAL_PEER_DESC,
                                 SettingsValues::PRIVACY_LOCAL_PEER_DESC_DEFAULT)
                          .toBool();
    sessParams.settings.set_bool(lt::settings_pack::enable_lsd, lsdEnabled);
}

void SessionManager::saveResumes()
{
    for (auto &torrent : m_torrentHandles)
    {
        if (torrent.isValid() && torrent.isNeedSaveData())
        {
            torrent.saveResumeData();
        }
    }
}

void SessionManager::eventLoop()
{
    std::vector<lt::alert *> alerts;
    m_session->pop_alerts(&alerts);
    for (auto *alert : alerts)
    {
        if (auto *finished_alert = lt::alert_cast<lt::torrent_finished_alert>(alert))
        {
            handleFinishedAlert(finished_alert);
        }
        else if (auto *statusAlert = lt::alert_cast<lt::state_update_alert>(alert))
        {
            handleStateUpdateAlert(statusAlert);
        }
        else if (auto *metadataReceivedAlert = lt::alert_cast<lt::metadata_received_alert>(alert))
        {
            handleMetadataReceived(metadataReceivedAlert);
        }
        else if (auto *resumeDataAlert = lt::alert_cast<lt::save_resume_data_alert>(alert))
        {
            handleResumeDataAlert(resumeDataAlert);
        }
        else if (auto *addTorrentAlert = lt::alert_cast<lt::add_torrent_alert>(alert))
        {
            handleAddTorrentAlert(addTorrentAlert);
        }
        else if (auto *stateAlert = lt::alert_cast<lt::session_stats_alert>(alert))
        {
            handleSessionStatsAlert(stateAlert);
        }
        else if (auto *torrentErrorAlert = lt::alert_cast<lt::torrent_error_alert>(alert))
        {
            handleTorrentErrorAlert(torrentErrorAlert);
        }
        else if (auto *moveFailedAlert = lt::alert_cast<lt::storage_moved_failed_alert>(alert))
        {
            handleStorageMoveFailedAlert(moveFailedAlert);
        }
        else if (auto *renameFileFailedAlert = lt::alert_cast<lt::file_rename_failed_alert>(alert))
        {
            handleFileRenameFailedAlert(renameFileFailedAlert);
        }
    }

    m_session->post_torrent_updates();
    m_session->post_session_stats(); // Needed for graphs
    updateProperties();

}

void SessionManager::setLoopDuration(int ms)
{
    assert(ms > 0 && ms < 10000);
    m_alertTimer.stop();
    m_alertTimer.start(ms);
}

// HERE: Separate function for each emit thingy
void SessionManager::updateProperties()
{
    if (m_currentTorrentId.has_value())
    {
        auto  currentTorrentId = m_currentTorrentId.value();
        auto &handle           = m_torrentHandles[currentTorrentId];
        {
            if (handle.isValid())
            {
                // Update peers
                updatePeersProp(handle);

                // Update trackers
                updateTrackersProp(handle);

                // Update files
                updateFilesProp(handle);

                // update url seeds
                updateUrlProp(handle);
            }
            else
            { // if its not valid something is wrong
                m_currentTorrentId = std::nullopt;
            }
        }
    }
    else
    {
        // clear stats
        emitClearSignals();
    }
}

void SessionManager::updatePeersProp(TorrentHandle &handle)
{
    std::vector<lt::peer_info> peers = handle.getPeerInfo();
    emit                       peerInfo(handle.id(), peers);
}

void SessionManager::updateTrackersProp(TorrentHandle &handle)
{
    auto           ltTrackers = handle.getTrackers();
    QList<Tracker> trackers;

    const bool hasV2   = handle.handle().info_hashes().has_v2();
    const auto version = hasV2 ? lt::protocol_version::V2 : lt::protocol_version::V1;

    for (const auto &tracker : ltTrackers)
    {
        Tracker tr{};
        tr.url  = QString::fromStdString(tracker.url);
        tr.tier = tracker.tier;
        for (const auto &ep : tracker.endpoints)
        {
            if (ep.enabled)
            {
                tr.isWorking = true;
            }
            const lt::announce_infohash ihash = ep.info_hashes[version];
            tr.seeds   = ihash.scrape_complete == -1 ? 0 : ihash.scrape_complete;
            tr.leeches = ihash.scrape_incomplete == -1 ? 0 : ihash.scrape_incomplete;
            tr.message = QString::fromStdString(ihash.message);
        }

        trackers.append(std::move(tr));
    }
    emit trackersInfo(trackers);
}

void SessionManager::updateFilesProp(TorrentHandle &handle)
{
    auto fileListFromTorrentInfo = [](const TorrentHandle                    &handle,
                                      std::shared_ptr<const lt::torrent_info> tinfo) -> QList<File>
    {
        const auto &files     = tinfo->files();
        auto        num_files = files.num_files();

        QList<File> result;
        auto        fileProgresses = handle.handle().file_progress();
        for (auto i = 0; i < num_files; ++i)
        {
            // auto fileSize = files.file_size(i);
            File file;
            file.id         = i;
            file.isEnabled  = handle.handle().file_priority(i) != lt::dont_download;
            file.filename   = QString::fromStdString(files.file_path(i));
            file.filesize   = files.file_size(i);
            file.downloaded = fileProgresses[i];
            file.priority   = handle.handle().file_priority(i);
            result.append(std::move(file));
        }
        return result;
    };

    if (auto tinfo = handle.handle().torrent_file())
    {
        auto files = fileListFromTorrentInfo(handle, tinfo);
        emit filesInfo(files);
    }
}

void SessionManager::updateUrlProp(TorrentHandle &handle)
{
    auto urlSeeds = handle.handle().url_seeds();
    emit urlSeedsInfo(urlSeeds);
}

void SessionManager::updateTorrent(TorrentHandle                    &torrentHandle,
                                   const libtorrent::torrent_status &status)
{
    bool   isPaused = torrentHandle.isPaused();
    double progress = std::ceil((static_cast<double>(status.total_wanted_done) /
                                 static_cast<double>(status.total_wanted) * 100.0) *
                                100) /
                      100.0;
    torrentHandle.resetCategory(); // sync category justin case

    double ratio;
    if (status.all_time_download < std::numeric_limits<double>::epsilon())
    {
        ratio = 0.0;
    }
    else
    {
        ratio = std::ceil(status.all_time_upload / static_cast<double>(status.all_time_download) *
                          100.0) /
                100.0;
    }
    Torrent torrent = {
        .id        = torrentHandle.id(),
        .category  = torrentHandle.getCategory(), // Default category is All,
        .name      = QString::fromStdString(status.name),
        .size      = status.total_wanted,
        .progress  = progress,
        .status    = !isPaused ? torrentStateToString(status.state) : "Stopped",
        .seeds     = status.num_seeds,
        .peers     = status.num_peers,
        .downSpeed = status.download_payload_rate, // count only pieces, without protocol stuff
        .upSpeed   = status.upload_rate,
        .ratio     = ratio,
        .eta       = status.download_rate == 0
                         ? -1
                         : (status.total_wanted - status.total_wanted_done) / status.download_rate,
    };
    emit torrentUpdated(torrent);
}

void SessionManager::handleFinishedAlert(libtorrent::torrent_finished_alert *alert)
{
    auto handles = m_torrentHandles.values();
    auto pos     = std::find_if(handles.begin(), handles.end(),
                                [alert](const auto &handle) { return handle.id() == alert->handle.id(); });

    pos->saveResumeData(); // Without it it does weird stuff
    pos->setCategory(Categories::SEEDING);
    emit torrentFinished(alert->handle.id(), alert->handle.status());
}

void SessionManager::handleStateUpdateAlert(libtorrent::state_update_alert *alert)
{
    auto statuses  = alert->status;
    auto currentId = m_currentTorrentId.has_value() ? m_currentTorrentId.value() : -1;
    for (auto &status : statuses)
    {
        auto &handle = status.handle;

        if (handle.id() == currentId)
        {
            updateGeneralProperty(handle);
        }
        handleStatusUpdate(status, handle);
    }
}

void SessionManager::updateGeneralProperty(const lt::torrent_handle &handle)
{
    auto         status = handle.status();
    InternetInfo iInfo;
    iInfo.activeTime = status.active_duration.count();
    iInfo.downloaded = status.all_time_download; // dependent on .fastresum
    iInfo.downSpeed  = status.download_rate;
    iInfo.downLimit  = handle.download_limit();

    iInfo.eta = status.download_rate == 0
                    ? -1
                    : (status.total_wanted - status.total_wanted_done) / status.download_rate;

    iInfo.uploaded = status.all_time_upload; // dependent on .fastresum
    iInfo.upSpeed  = status.upload_rate;
    iInfo.upLimit  = handle.upload_limit();

    if (status.all_time_download < std::numeric_limits<double>::epsilon())
    {
        iInfo.ratio = 0.0;
    }
    else
    {
        iInfo.ratio = std::ceil(status.all_time_upload /
                                static_cast<double>(status.all_time_download) * 100.0) /
                      100.0;
    }

    iInfo.connections = status.num_connections;
    iInfo.seeds       = status.num_seeds;
    iInfo.peers       = status.num_peers;

    TorrentInfo tInfo;
    tInfo.completedTime = status.completed_time;
    tInfo.size          = status.total_wanted;
    tInfo.startTime     = status.added_time;

    tInfo.hashV1 = utils::toHex(handle.info_hashes().v1.to_string());

    auto hashV2  = handle.info_hashes().v2;
    tInfo.hashV2 = utils::toHex(hashV2.is_all_zeros() ? "" : hashV2.to_string());

    tInfo.savePath = QString::fromStdString(status.save_path);

    {
        auto t_file   = handle.torrent_file();
        tInfo.comment = t_file ? QString::fromStdString(t_file->comment()) : "";
    }

    tInfo.piecesCount = status.pieces.size();

    auto tfile = handle.torrent_file();
    if (tfile)
    {
        tInfo.pieceSize = tfile->piece_length();
    }

    emit generalInfo(tInfo, iInfo);

    // Update pieces bar
    std::vector<int> downloadingPiecesIndices;

    std::vector<lt::partial_piece_info> pieces;
    handle.get_download_queue(pieces);

    for (const auto &pcs : pieces)
    {
        if (pcs.finished && pcs.finished < pcs.blocks_in_piece)
        { // finished more than 1, finished == blocks means finished
            downloadingPiecesIndices.push_back(pcs.piece_index);
        }
    }

    emit pieceBarInfo(status.pieces, downloadingPiecesIndices);
}

void SessionManager::handleStatusUpdate(const lt::torrent_status         &status,
                                        const libtorrent::torrent_handle &handle)
{
    auto &torrentHandle = m_torrentHandles[handle.id()];
    if (!torrentHandle.isValid())
        return;
    updateTorrent(torrentHandle, status);
}

void SessionManager::handleMetadataReceived(libtorrent::metadata_received_alert *alert)
{
    detail::writeTorrentFile(alert->handle.torrent_file());
}

void SessionManager::handleResumeDataAlert(libtorrent::save_resume_data_alert *alert)
{
    if (alert->handle.is_valid() && alert->handle.torrent_file())
    {
        auto resumeDataBuf = lt::write_resume_data_buf(alert->params);
        detail::saveResumeData(alert->handle.torrent_file(), resumeDataBuf);
    }
}

void SessionManager::handleAddTorrentAlert(libtorrent::add_torrent_alert *alert)
{
    auto &torrent_handle = alert->handle;
    m_torrentHandles.insert(torrent_handle.id(), TorrentHandle{torrent_handle});
    m_torrentHandles[torrent_handle.id()].setCategory(
        Categories::RUNNING); // this is gonna be handy if im ever gonna let user add torrents as
                              // paused

    // No point setting status fields, since they are zeroed and will be filled on status alert
    Torrent torrent = {
                       .id = torrent_handle.id(),
                       .category = QString{},
                       .name = QString::fromStdString(torrent_handle.status().name),
                       .size = 0,
                       .progress = 0.0,
                       .status =torrentStateToString(torrent_handle.status().state),
                       .seeds =0,
                       .peers = 0,
                       .downSpeed = 0,
                       .upSpeed = 0,
                       .ratio = 0,
                       .eta = 0
                    };

    emit torrentAdded(torrent);
}

void SessionManager::handleSessionStatsAlert(libtorrent::session_stats_alert *alert)
{
    static auto last = std::chrono::high_resolution_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - last).count();
    double multiplier = static_cast<double>(std::chrono::milliseconds(1000).count()) / (diff == 0 ? std::chrono::milliseconds(1000).count() : diff);

    const auto &counters         = alert->counters();
    auto        recvPayloadBytes = counters[lt::counters::recv_payload_bytes];
    auto        newRecv          = recvPayloadBytes - lastSessionRecvPayloadBytes;
    newRecv = newRecv * multiplier;
    lastSessionRecvPayloadBytes  = recvPayloadBytes;

    auto uploadPayloadBytes       = counters[lt::counters::sent_bytes];
    auto newUpload                = uploadPayloadBytes - lastSessionUploadPayloadBytes;
    newUpload = newUpload * multiplier;
    lastSessionUploadPayloadBytes = uploadPayloadBytes;

    emit chartPoint(newRecv, newUpload);
    last = std::chrono::high_resolution_clock::now();
}

void SessionManager::handleTorrentErrorAlert(libtorrent::torrent_error_alert *alert)
{
    qDebug() << "Torrent Error Alert:" << alert->message();
    auto  id            = alert->handle.id();
    auto &torrentHandle = m_torrentHandles[id];
    torrentHandle.setCategory(Categories::FAILED);
}

void SessionManager::handleStorageMoveFailedAlert(libtorrent::storage_moved_failed_alert *alert)
{
    emit torrentFileMoveFailed(QString::fromStdString(alert->message()), alert->torrent_name());
}

void SessionManager::handleFileRenameFailedAlert(libtorrent::file_rename_failed_alert *alert)
{
    emit torrentFileMoveFailed(QString::fromStdString(alert->message()), alert->torrent_name());
}

void SessionManager::loadResumes()
{
    auto basePath     = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + Dirs::STATE;
    QDir dir{stateDirPath};
    auto entries = dir.entryList(QDir::Filter::Files);
    for (auto &entry : entries)
    {
        if (!entry.endsWith(".fastresume"))
        {
            qWarning() << "Wrong formatted file in state directory";
            continue;
        }

        QFile file{stateDirPath + QDir::separator() + entry};
        if (file.open(QIODevice::ReadOnly))
        {
            auto buffer = file.readAll();
            auto params = lt::read_resume_data(buffer);
            // bool isPaused =
            //     (m_handle.flags() & (lt::torrent_flags::paused)) == lt::torrent_flags::paused ?
            //     true
            //                                                                                   :
            //                                                                                   false;
            // if (isPaused)
            // {
            //     params.flags &= ~lt::torrent_flags::auto_managed;
            // }
            m_session->async_add_torrent(std::move(params));
        }
    }
}

void SessionManager::resetSessionParams() { shouldResetParams = true; }

void SessionManager::setDownloadLimit(int value)
{
    lt::settings_pack newSettings = m_session->get_settings();
    if (int oldVal = newSettings.get_int(lt::settings_pack::download_rate_limit); oldVal != value)
    {
        newSettings.set_int(lt::settings_pack::download_rate_limit, value);
        m_session->apply_settings(std::move(newSettings));
    }
}

void SessionManager::setUploadLimit(int value)
{
    lt::settings_pack newSettings = m_session->get_settings();
    if (int oldVal = newSettings.get_int(lt::settings_pack::upload_rate_limit); oldVal != value)
    {
        newSettings.set_int(lt::settings_pack::upload_rate_limit, value);
        m_session->apply_settings(std::move(newSettings));
    }
}

void SessionManager::setListenPort(unsigned short newPort)
{
    lt::settings_pack newSettings         = m_session->get_settings();
    QString           listeningInterfaces = QString{"0.0.0.0:%1,[::]:%1"}.arg(newPort);
    if (auto oldVal =
            QString::fromStdString(newSettings.get_str(lt::settings_pack::listen_interfaces));
        oldVal != listeningInterfaces)
    {
        newSettings.set_str(lt::settings_pack::listen_interfaces,
                            listeningInterfaces.toStdString());
        m_session->apply_settings(std::move(newSettings));
    }
}

void SessionManager::setListenProtocol(int protocolType)
{
    lt::settings_pack newSettings = m_session->get_settings();
    switch (protocolType)
    {
    case SettingsValues::LISTENING_PROTOCOL_TCP_AND_UTP:
    {
        newSettings.set_bool(lt::settings_pack::enable_outgoing_tcp, true);
        newSettings.set_bool(lt::settings_pack::enable_incoming_tcp, true);
        newSettings.set_bool(lt::settings_pack::enable_outgoing_utp, true);
        newSettings.set_bool(lt::settings_pack::enable_incoming_utp, true);
        break;
    }
    case SettingsValues::LISTENING_PROTOCOL_TCP:
    {
        newSettings.set_bool(lt::settings_pack::enable_outgoing_tcp, true);
        newSettings.set_bool(lt::settings_pack::enable_incoming_tcp, true);
        newSettings.set_bool(lt::settings_pack::enable_outgoing_utp, false);
        newSettings.set_bool(lt::settings_pack::enable_incoming_utp, false);
        break;
    }
    case SettingsValues::LISTENING_PROTOCOL_UTP:
    {
        newSettings.set_bool(lt::settings_pack::enable_outgoing_tcp, false);
        newSettings.set_bool(lt::settings_pack::enable_incoming_tcp, false);
        newSettings.set_bool(lt::settings_pack::enable_outgoing_utp, true);
        newSettings.set_bool(lt::settings_pack::enable_incoming_utp, true);
        break;
    }
    }
    m_session->apply_settings(newSettings);
}

void SessionManager::setMaxNumberOfConnections(int value)
{
    lt::settings_pack newSettings = m_session->get_settings();
    if (int oldVal = newSettings.get_int(lt::settings_pack::connections_limit); oldVal != value)
    {
        newSettings.set_int(lt::settings_pack::connections_limit, value);
        m_session->apply_settings(newSettings);
    }
}

void SessionManager::setDht(bool value)
{
    lt::settings_pack newSettings = m_session->get_settings();
    if (bool oldVal = newSettings.get_bool(lt::settings_pack::enable_dht); oldVal != value)
    {
        newSettings.set_bool(lt::settings_pack::enable_dht, value);
        m_session->apply_settings(newSettings);
    }
}

void SessionManager::setLsd(bool value)
{
    lt::settings_pack newSettings = m_session->get_settings();
    if (bool oldVal = newSettings.get_bool(lt::settings_pack::enable_lsd); oldVal != value)
    {
        newSettings.set_bool(lt::settings_pack::enable_lsd, value);
        m_session->apply_settings(newSettings);
    }
}

void SessionManager::setUpnp(bool value)
{
    lt::settings_pack newSettings = m_session->get_settings();
    if (bool oldVal = newSettings.get_bool(lt::settings_pack::enable_upnp); oldVal != value) {
        newSettings.set_bool(lt::settings_pack::enable_upnp, value);
        newSettings.set_bool(lt::settings_pack::enable_natpmp, value); // Enable this as well, since this is kinda the same thing in terms of functionality
        m_session->apply_settings(newSettings);
    }
}

void SessionManager::changeFilePriority(std::uint32_t id, int fileIndex, int priority)
{
    auto &handle = m_torrentHandles[id];
    // No need to unset auto_managed, since it onyl takes care of pausing/unpausing and some other
    // stuff, but definitely not files' priorities
    handle.setFilePriority(fileIndex, priority);
}

void SessionManager::renameFile(uint32_t id, int fileIndex, const QString &newName)
{
    auto &handle = m_torrentHandles[id];
    handle.renameFile(fileIndex, newName);
}

void SessionManager::banPeers(const QList<boost::asio::ip::address> &bannablePeers)
{
    lt::ip_filter filter = m_session->get_ip_filter();
    for (const auto &peerAddr : bannablePeers)
    {
        filter.add_rule(peerAddr, peerAddr, lt::ip_filter::blocked);
    }
    m_session->set_ip_filter(std::move(filter));
}

void SessionManager::setIpFilter(const QList<boost::asio::ip::address> &addrs)
{
    lt::ip_filter filter{};
    for (const auto &addr : addrs)
    {
        filter.add_rule(addr, addr, lt::ip_filter::blocked);
    }
    m_session->set_ip_filter(std::move(filter));
}

lt::ip_filter::filter_tuple_t SessionManager::getIpFilter() const
{
    lt::ip_filter filter      = m_session->get_ip_filter();
    auto          bannedPeers = filter.export_filter();
    return bannedPeers;
}

void SessionManager::addPeerToTorrent(std::uint32_t id, const boost::asio::ip::tcp::endpoint &ep)
{
    assert(id < m_torrentHandles.size());
    m_torrentHandles[id].connectToPeer(ep);
}

void SessionManager::addPeersToTorrent(std::uint32_t                                id,
                                       const QList<boost::asio::ip::tcp::endpoint> &eps)
{
    assert(id < m_torrentHandles.size());
    auto &torrentHandle = m_torrentHandles[id];
    for (const auto &ep : eps)
    {
        torrentHandle.connectToPeer(ep);
    }
}

void SessionManager::setCurrentTorrentId(std::optional<uint32_t> value)
{
    if (m_currentTorrentId.has_value() && m_currentTorrentId.value() == value)
    {
        return;
    }
    m_currentTorrentId = value;
}

void SessionManager::forceUpdateProperties()
{
    auto &torrentHandle = m_torrentHandles[m_currentTorrentId.value()];
    updatePeersProp(torrentHandle);
    updateGeneralProperty(torrentHandle.handle());
    updateTrackersProp(torrentHandle);
    updateUrlProp(torrentHandle);
    updateFilesProp(torrentHandle);
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
    params.ti        = std::move(torrent_info);
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

bool SessionManager::addTorrentByTorrentInfo(std::shared_ptr<const libtorrent::torrent_info> ti,
                                             const QList<lt::download_priority_t> &filePriorities,
                                             QStringView                           outputDir)
{
    lt::add_torrent_params params{};
    detail::writeTorrentFile(ti);

    params.ti = std::make_shared<lt::torrent_info>(*ti);

    std::vector<lt::download_priority_t> priorities{filePriorities.constBegin(),
                                                    filePriorities.end()};
    params.file_priorities = std::move(priorities);
    params.save_path       = outputDir.toString().toStdString();
    return addTorrent(params);
}

bool SessionManager::isTorrentPaused(const uint32_t id) const
{
    auto &torrentHandle = m_torrentHandles[id];
    return torrentHandle.isPaused();
}

void SessionManager::pauseTorrent(const uint32_t id) { m_torrentHandles[id].pause(); }

void SessionManager::resumeTorrent(const uint32_t id) { m_torrentHandles[id].resume(); }

bool SessionManager::removeTorrent(const uint32_t id, bool removeWithContents)
{
    if (m_currentTorrentId.has_value() && id == m_currentTorrentId.value())
    {
        m_currentTorrentId = std::nullopt;
    }

    auto &torrentHandle = m_torrentHandles[id];

    m_session->remove_torrent(torrentHandle.handle(), removeWithContents ? lt::session::delete_files
                                                                         : lt::remove_flags_t{});
    // Delete .fastresume and .torrent
    auto basePath     = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto torrentsPath = basePath + QDir::separator() + Dirs::TORRENTS;

    auto hashString  = torrentHandle.bestHashAsString();
    auto torrentFile = torrentsPath + QDir::separator() + hashString + FileEnding::DOT_TORRENT;

    if (!QFile::remove(torrentFile))
    {
        qWarning() << "Could not remove .torrent file";
        return false;
    }

    auto statePath = basePath + QDir::separator() + Dirs::STATE;
    auto stateFile = statePath + QDir::separator() + hashString + FileEnding::DOT_FASTRESUME;
    if (!QFile::remove(stateFile))
    {
        qWarning() << "Could not remove .fastresume file";
        return false;
    }

    m_torrentHandles.remove(id);
    emit torrentDeleted(id);
    return true;
}

void SessionManager::setTorrentDownloadLimit(const uint32_t id, int newLimit)
{
    m_torrentHandles[id].setDownloadLimit(newLimit);
}

void SessionManager::setTorrentUploadLimit(const uint32_t id, int newLimit)
{
    m_torrentHandles[id].setUploadLimit(newLimit);
}

void SessionManager::setTorrentSavePath(const std::uint32_t id, const QString &newPath)
{
    // TODO: Might wanna handle failed/success alrts
    m_torrentHandles[id].moveStorage(newPath);
}

void SessionManager::setTorrentMaxConn(const uint32_t id, int newValue)
{
    m_torrentHandles[id].setMaxConn(newValue);
}

void SessionManager::setTorrentDht(const uint32_t id, bool enabled)
{
    m_torrentHandles[id].setDht(enabled);
}
void SessionManager::setTorrentPex(const std::uint32_t id, bool enabled)
{
    m_torrentHandles[id].setPex(enabled);
}
void SessionManager::setTorrentLsd(const std::uint32_t id, bool enabled)
{
    m_torrentHandles[id].setLsd(enabled);
}

bool SessionManager::addTorrent(libtorrent::add_torrent_params params)
{
    if (isTorrentExists(params.info_hashes.get_best().is_all_zeros()
                            ? params.ti->info_hashes().get_best()
                            : params.info_hashes.get_best()))
    {
        return false;
    }
    // Set per torrent parameters
    detail::setupTorrentSettings(params);

    m_session->async_add_torrent(std::move(params));
    return true;
}

bool SessionManager::isTorrentExists(const lt::sha1_hash &hash) const
{
    auto handles     = m_torrentHandles.values();
    auto torrentIter = std::find_if(handles.begin(), handles.end(), [&](const TorrentHandle &value)
                                    { return value.bestHash() == hash; });
    return torrentIter != handles.end();
}

namespace detail
{
void writeTorrentFile(std::shared_ptr<const libtorrent::torrent_info> ti)
{
    auto basePath     = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath = basePath + QDir::separator() + Dirs::TORRENTS;

    auto torrentFilePath = stateDirPath + QDir::separator() +
                           utils::toHex(ti->info_hashes().get_best().to_string()) +
                           FileEnding::DOT_TORRENT;
    QFile file{torrentFilePath};

    if (file.open(QIODevice::WriteOnly))
    {
        lt::create_torrent ci{*ti};
        auto               buf = ci.generate_buf();
        file.write(buf.data(), buf.size());

        file.flush();
        file.close();
    }
}

void saveResumeData(std::shared_ptr<const libtorrent::torrent_info> ti,
                    const std::vector<char>                        &buf)
{
    auto basePath      = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto stateDirPath  = basePath + QDir::separator() + Dirs::STATE;
    auto stateFilePath = stateDirPath + QDir::separator() +
                         utils::toHex(ti->info_hashes().get_best().to_string()) +
                         FileEnding::DOT_FASTRESUME;
    QFile file{stateFilePath};
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(buf.data(), buf.size());

        file.flush();
        file.close();
    }
}

std::vector<char> readFile(const char *filename)
{
    QFile file{filename};
    if (file.open(QIODevice::ReadOnly))
    {
        auto bytes = file.readAll();
        return std::vector<char>{bytes.begin(), bytes.end()};
    }
    return {};
}

void setupTorrentSettings(libtorrent::add_torrent_params &params)
{
    QSettings settings;
    int       numOfConPt = settings
                         .value(SettingsNames::LIMITS_MAX_NUM_OF_CONNECTIONS_PT,
                                SettingsValues::LIMITS_MAX_NUM_OF_CONNECTIONS_PT_DEFAULT)
                         .toInt();
    params.max_connections = numOfConPt;

    bool enablePeerEx = settings
                            .value(SettingsNames::PRIVACY_PEEREX_ENABLED,
                                   SettingsValues::PRIVACY_PEEREX_ENABLED_DEFAULT)
                            .toBool();
    if (!enablePeerEx)
    {
        params.flags |= lt::torrent_flags::disable_pex;
    }
}

} // namespace detail
