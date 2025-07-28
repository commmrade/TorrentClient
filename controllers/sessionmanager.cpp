#include "sessionmanager.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

SessionManager::SessionManager(QObject *parent)
    : QObject{parent}
{

    // Setting up session and its settings
    auto sessionFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + SESSION_FILENAME;
    auto sessionFileContents = readFile(sessionFilePath.toUtf8().constData());
    lt::session_params sess_params;
    if (sessionFileContents.empty()) {
        sess_params.settings.set_int(lt::settings_pack::alert_mask,
            lt::alert_category::status |
            lt::alert_category::error |
            lt::alert_category::storage);

        // sess_params.settings.set_int(lt::settings_pack::download_rate_limit, 1 * 1024 * 1024); // Limti download speed
    } else {
        sess_params = std::move(lt::read_session_params(sessionFileContents));
    }
    m_session = std::make_unique<lt::session>(std::move(sess_params));

    connect(&m_alertTimer, &QTimer::timeout, this, &SessionManager::eventLoop);
    m_alertTimer.start(100);
    connect(&m_resumeDataTimer, &QTimer::timeout, this, [this] {
        for (auto& torrent : m_torrentHandles) {
            if (torrent.need_save_resume_data() && torrent.is_valid()) {
                torrent.save_resume_data();
            }
        }
    });
    m_resumeDataTimer.start(2000);
    loadResumes();
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

void SessionManager::addTorrentByFilename(QStringView filepath)
{
    auto torrent_info = std::make_shared<lt::torrent_info>(filepath.toUtf8().toStdString());
    lt::add_torrent_params params{};

    writeTorrentFile(torrent_info);

    params.ti = std::move(torrent_info);
    addTorrent(std::move(params));
}

void SessionManager::addTorrentByMagnet(QString magnetURI)
{
    auto params = lt::parse_magnet_uri(magnetURI.toStdString());
    addTorrent(std::move(params));
}

void SessionManager::eventLoop()
{
    std::vector<lt::alert*> alerts;
    m_session->pop_alerts(&alerts);
    // qDebug() << "process alerts";

    for (auto* alert : alerts) {
        if (auto* finished_alert = lt::alert_cast<lt::torrent_finished_alert>(alert)) {
            qDebug() << "finished downloading";
            auto pos = std::find_if(m_torrentHandles.begin(), m_torrentHandles.end(), [finished_alert](auto&& handle) {
                return handle.id() == finished_alert->handle.id();
            });
            auto name = QString::fromStdString(lt::aux::to_hex(finished_alert->handle.info_hashes().get_best().to_string()));
            m_torrentHandles.remove(name);
        }
        if (auto* statusAlert = lt::alert_cast<lt::state_update_alert>(alert)) {
            auto statuses = statusAlert->status;
            for (auto status : statuses) {
                qDebug() << "Speed"
                         << status.download_rate / 1024.0 / 1024.0 << "MB/s" << "Name" << status.name << "Got: "
                         << status.total_done / 1024.0 / 1024.0 << "MB out of"
                         << status.total_wanted / 1024.0 / 1024.0 << "MB";
            }
        }
        if (auto* metadataReceivedAlert = lt::alert_cast<lt::metadata_received_alert>(alert)) {
            // Need to backup the torrent file for a download that was started via magnet uri (.torrent already has all necessary metadata)
            writeTorrentFile(metadataReceivedAlert->handle.torrent_file());
        }
        if (auto* resumeDataAlert = lt::alert_cast<lt::save_resume_data_alert>(alert)) {
            if (resumeDataAlert->handle.torrent_file()) {
                auto resumeDataBuf = lt::write_resume_data_buf(resumeDataAlert->params);
                saveResumeData(resumeDataAlert->handle.torrent_file(), resumeDataBuf);
            }
        }
    }
    // For example, if its finish alert, find torrent handle by id and remove it
    m_session->post_torrent_updates();
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
            m_torrentHandles.insert(QString::fromStdString(lt::aux::to_hex(torrent_handle.info_hashes().get_best().to_string())), std::move(torrent_handle));
        }
    }
}

void SessionManager::addTorrent(libtorrent::add_torrent_params params)
{
    auto appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto torrentsSaveDir = appDataPath + QDir::separator() + "torrents";
    params.save_path = torrentsSaveDir.toStdString();
    // TODO: Make it async later
    auto torrent_handle = m_session->add_torrent(params);
    // m_torrentHandles.append(torrent_handle);
    m_torrentHandles.insert(QString::fromStdString(lt::aux::to_hex(torrent_handle.info_hashes().get_best().to_string())), torrent_handle);
}
