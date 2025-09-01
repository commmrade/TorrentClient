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
#include "utils.h"

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

    m_fetcher = new MetadataFetcher{params};
    connect(m_fetcher, &MetadataFetcher::sizeReady, this, &SaveTorrentDialog::setSize);
    m_fetcher->start();

    // Simulating detached (hack)
    connect(m_fetcher, &QThread::finished, m_fetcher, &QThread::deleteLater);

    QSettings settings;
    auto savePath = settings.value(SettingsValues::SESSION_DEFAULT_SAVE_LOCATION, QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    ui->savePathLineEdit->setText(savePath);
}

SaveTorrentDialog::~SaveTorrentDialog()
{
    if (m_fetcher) {
        m_fetcher->stopRunning();
    }
    delete ui;
}

QString SaveTorrentDialog::getSavePath() const
{
    return ui->savePathLineEdit->text();
}

void SaveTorrentDialog::setSize(int64_t bytes)
{
    auto bytesStr = bytesToHigher(bytes);
    ui->sizeInfo->setText(bytesStr);
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

