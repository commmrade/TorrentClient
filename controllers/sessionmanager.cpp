#include "sessionmanager.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include "torrent.h"

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
    } else {
        sessParams = std::move(lt::read_session_params(sessionFileContents));
    }
    return sessParams;
}


void SessionManager::saveResumes()
{
    for (auto& torrent : m_torrentHandles) {
        if (torrent.need_save_resume_data() && torrent.is_valid()) {
            torrent.save_resume_data();
        }
    }
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
    }
    m_session->post_torrent_updates();
}

void SessionManager::handleFinishedAlert(libtorrent::torrent_finished_alert *alert)
{
    auto pos = std::find_if(m_torrentHandles.begin(), m_torrentHandles.end(), [alert](auto&& handle) {
        return handle.id() == alert->handle.id();
    });
    pos->save_resume_data(); // Otherwise it's behaving kinda weird
    emit torrentFinished(alert->handle.id(), alert->handle.status());
}

void SessionManager::handleStateUpdateAlert(libtorrent::state_update_alert *alert)
{
    auto statuses = alert->status;
    for (auto status : statuses) {
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
            auto torrent_handle = m_session->add_torrent(std::move(params));
            m_torrentHandles.insert(QString::fromStdString(lt::aux::to_hex(torrent_handle.info_hashes().get_best().to_string())), torrent_handle);

            Torrent torrent = {
                torrent_handle.id(),
                QString::fromStdString(torrent_handle.status().name),
                "0 MB",
                0.0,
                torrentStateToString(torrent_handle.status().state),
                0,
                0,
                "0 MB/s",
                "0 MB/s"
            };
            emit torrentAdded(torrent);
        }
    }
}


void SessionManager::addTorrentByFilename(QStringView filepath)
{
    auto torrent_info = std::make_shared<lt::torrent_info>(filepath.toUtf8().toStdString());
    lt::add_torrent_params params{};

    writeTorrentFile(torrent_info);

    params.ti = std::move(torrent_info);
    addTorrent(params);
}

void SessionManager::addTorrentByMagnet(QString magnetURI)
{
    auto params = lt::parse_magnet_uri(magnetURI.toStdString());
    addTorrent(std::move(params));
}

void SessionManager::addTorrent(libtorrent::add_torrent_params params)
{
    auto appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto torrentsSaveDir = appDataPath + QDir::separator() + "torrents";
    params.save_path = torrentsSaveDir.toStdString();

    // TODO: Make it async later
    auto torrent_handle = m_session->add_torrent(params);
    m_torrentHandles.insert(QString::fromStdString(lt::aux::to_hex(torrent_handle.info_hashes().get_best().to_string())), torrent_handle);

    // No point setting status fields, since they are zeroed and will be filled on status alert
    Torrent torrent = {
        torrent_handle.id(),
        QString::fromStdString(torrent_handle.status().name),
        "0 MB",
        0.0,
        torrentStateToString(torrent_handle.status().state),
        0,
        0,
        "0 MB/s",
        "0 MB/s"
    };

    emit torrentAdded(torrent);
}

void SessionManager::handleStatusUpdate(const lt::torrent_status& status, const libtorrent::torrent_handle &handle)
{
    Torrent torrent = {
        handle.id(),
        QString::fromStdString(status.name),
        QString::number(status.total_wanted / 1024.0 / 1024.0) + " MB",
        std::ceil(((status.total_done / 1024.0 / 1024.0) / (status.total_wanted / 1024.0 / 1024.0) * 100.0) * 100) / 100.0,
        torrentStateToString(status.state),
        status.num_seeds,
        status.num_peers,
        QString::number(std::ceil(status.download_rate / 1024.0 / 1024.0 * 100.0) / 100.0) + " MB/s",
        QString::number(std::ceil(status.upload_rate / 1024.0 / 1024.0 * 100.0) / 100.0) + " MB/s",
    };
    emit torrentUpdated(torrent);
}
