#include "savetorrentdialog.h"
#include "ui_savetorrentdialog.h"
#include <libtorrent/torrent_info.hpp>
#include <QSettings>
#include "settingsvalues.h"
#include <QStandardPaths>
#include <QFileDialog>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/hex.hpp>
// #include <libtorrent/libtorrent.hpp>
#include <QThread>

SaveTorrentDialog::SaveTorrentDialog(torrent_file_tag, const QString& torrentPath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SaveTorrentDialog)
{
    ui->setupUi(this);

    lt::torrent_info info{torrentPath.toStdString()};
    auto totalSize = info.total_size() / 1024.0 / 1024.0;
    ui->sizeInfo->setText(QString::number(totalSize) + " MB");

    QSettings settings;
    auto savePath = settings.value(SettingsValues::SESSION_DEFAULT_SAVE_LOCATION, QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    ui->savePathLineEdit->setText(savePath);
}

SaveTorrentDialog::SaveTorrentDialog(magnet_tag, const QString &magnetUri, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SaveTorrentDialog)
{
    ui->setupUi(this);
    ui->sizeInfo->setText("Fetching...");

    lt::add_torrent_params params = lt::parse_magnet_uri(magnetUri.toStdString());
    auto saveTorrentFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + "torrents" + QDir::separator() + QString::fromStdString(libtorrent::aux::to_hex(params.info_hashes.get_best().to_string()));
    params.save_path = saveTorrentFile.toStdString();

    std::thread{[this, params = std::move(params)]() mutable {
        lt::session_params sessParams;
        sessParams.settings.set_int(
        lt::settings_pack::alert_mask,
        lt::alert_category::status |
            lt::alert_category::error |
            lt::alert_category::storage
        );

        lt::session session{sessParams};
        auto torrentHandle = session.add_torrent(std::move(params));

        bool is_running {true};
        while (is_running) {
            std::vector<lt::alert*> alerts;

            session.pop_alerts(&alerts);
            for (auto* alert : alerts) {
                if (auto metadataRecAlert = lt::alert_cast<lt::metadata_received_alert>(alert)) {
                    auto totalSize = metadataRecAlert->handle.torrent_file()->total_size() / 1024.0 / 1024.0;

                    ui->sizeInfo->setText(QString::number(totalSize) + " MB");

                    metadataRecAlert->handle.pause();
                    is_running = false;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }}.detach();

    QSettings settings;
    auto savePath = settings.value(SettingsValues::SESSION_DEFAULT_SAVE_LOCATION, QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    ui->savePathLineEdit->setText(savePath);
}

SaveTorrentDialog::~SaveTorrentDialog()
{
    // if (thread) thread->join();
    delete ui;
}

QString SaveTorrentDialog::getSavePath() const
{
    return ui->savePathLineEdit->text();
}

void SaveTorrentDialog::on_changeSavePathButton_clicked()
{
    QString saveDir = QFileDialog::getExistingDirectory(this, "Choose a directory to save torrent in");
    if (!saveDir.isEmpty()) {
        ui->savePathLineEdit->setText(saveDir);
    }
}

void SaveTorrentDialog::tryLoadData()
{

}

