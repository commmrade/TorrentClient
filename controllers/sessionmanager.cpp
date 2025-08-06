#include "sessionmanager.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include "torrent.h"
#include <libtorrent/libtorrent.hpp>
#include <QSettings>
#include "settingsvalues.h"

SessionManager::SessionManager(QObject *parent)
    : QObject{parent}
{
    auto sessParams = loadSessionParams();
    m_session = std::make_unique<lt::session>(std::move(sessParams));

    connect(&m_alertTimer, &QTimer::timeout, this, &SessionManager::eventLoop);
    m_alertTimer.start(100);
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
    }

    if (m_currentTorrentId != -1) {
        auto& handle = m_torrentHandles[m_currentTorrentId];
        if (handle.isValid()) {
            std::vector<lt::peer_info> peers;
            // TODO: Should i make it async?
            m_torrentHandles[m_currentTorrentId].handle().get_peer_info(peers);
            emit peerInfo(m_currentTorrentId, std::move(peers));
        }
    } else {
        emit clearPeerInfo();
    }

    m_session->post_torrent_updates();
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
        handleStatusUpdate(status, handle);
    }
}

void SessionManager::handleMetadataReceived(libtorrent::metadata_received_alert *alert)
{
    writeTorrentFile(alert->handle.torrent_file());
}

void SessionManager::handleResumeDataAlert(libtorrent::save_resume_data_alert *alert)
{
    if (alert->handle.torrent_file()) {
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

void SessionManager::addTorrentByFilename(QStringView filepath, QStringView outputDir)
{
    auto torrent_info = std::make_shared<lt::torrent_info>(filepath.toUtf8().toStdString());
    lt::add_torrent_params params{};

    writeTorrentFile(torrent_info);

    params.ti = std::move(torrent_info);
    params.save_path = outputDir.toString().toStdString();
    addTorrent(params);
}

void SessionManager::addTorrentByMagnet(QString magnetURI, QStringView outputDir)
{
    auto params = lt::parse_magnet_uri(magnetURI.toStdString());
    qDebug() << "magnet torrent" << params.name;
    params.save_path = outputDir.toString().toStdString();
    addTorrent(std::move(params));
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

    auto hashString = QString{lt::aux::to_hex(torrentHandle.handle().info_hashes().get_best().to_string()).c_str()};
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

void SessionManager::addTorrent(libtorrent::add_torrent_params params)
{
    if (isTorrentExists(params.info_hashes.get_best().is_all_zeros() ? params.ti->info_hashes().get_best() : params.info_hashes.get_best())) {
        // TODO: Signal error adding torrent
        qDebug() << "here??";
        return;
    }
    m_session->async_add_torrent(std::move(params));
}

void SessionManager::handleStatusUpdate(const lt::torrent_status& status, const libtorrent::torrent_handle &handle)
{
    bool IsPaused = (status.flags & (lt::torrent_flags::paused)) == lt::torrent_flags::paused ? true : false;


    // debug
    // std::vector<lt::peer_info> peers;
    // handle.get_peer_info(peers);
    // for (const auto& peer : peers) {
    //     QString conType;
    //     if (peer.connection_type == lt::peer_info::standard_bittorrent) {
    //         conType = "BT";
    //     } else if (peer.connection_type == lt::peer_info::http_seed) {
    //         conType = "HTTP";
    //     } else {
    //         conType = "URL";
    //     }

    //     // { // Ban a peer
    //     //     lt::ip_filter filter = m_session->get_ip_filter();
    //     //     filter.add_rule(peer.ip.address(), peer.ip.address(), lt::ip_filter::blocked);
    //     //     m_session->set_ip_filter(std::move(filter));
    //     // }


    //     qDebug() << "Country:" << "TODO" << "Ip:" << peer.ip.address().to_string() <<
    //         "Port:" << peer.ip.port() << "Connection:" << conType <<
    //         "Client:" << peer.client << "Progress:" << peer.progress <<
    //         "Down speed:" << peer.down_speed / 1024.0 << "Up Speed:" << peer.up_speed <<
    //         "Downloaded:" << peer.total_download / 1024.0 << "Uploaded:" << peer.total_upload / 1024.0;
    // }
    //

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
        static_cast<std::uint64_t>(status.download_rate),
        // QString::number(std::ceil(status.upload_rate / 1024.0 / 1024.0 * 100.0) / 100.0) + " MB/s",
        static_cast<std::uint64_t>(status.upload_rate),
        status.download_rate == 0 ? -1 : static_cast<int>(status.total_wanted / status.download_rate),
    };
    emit torrentUpdated(torrent);
}

bool SessionManager::isTorrentExists(const lt::sha1_hash& hash) const
{
    auto handles = m_torrentHandles.values();
    auto torrentIter = std::find_if(handles.begin(), handles.end(), [&](const TorrentHandle& value) {
        qDebug() << lt::aux::to_hex(value.handle().info_hashes().get_best()) << lt::aux::to_hex(hash);
        return value.handle().info_hashes().get_best() == hash;
    });
    return torrentIter != handles.end();
}
