#include "savetorrentdialog.h"
#include "dirs.h"
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
#include <QThread>
#include "utils.h"
#include <QMessageBox>

SaveTorrentDialog::SaveTorrentDialog(torrent_file_tag, const QString& torrentPath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SaveTorrentDialog)
{
    ui->setupUi(this);

    lt::torrent_info info{torrentPath.toStdString()};

    setDataFromTi(info);

    QSettings settings;
    auto savePath = settings.value(SettingsValues::SESSION_DEFAULT_SAVE_LOCATION, QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    ui->savePathLineEdit->setText(savePath);
}

SaveTorrentDialog::SaveTorrentDialog(magnet_tag, const QString &magnetUri, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SaveTorrentDialog)
{
    ui->setupUi(this);
    ui->sizeInfo->setText(tr("Fetching..."));

    lt::add_torrent_params params = lt::parse_magnet_uri(magnetUri.toStdString());
    auto saveTorrentFile =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        QDir::separator() +
        Dirs::TORRENTS +
        QDir::separator() +
        utils::toHex(params.info_hashes.get_best().to_string());

    params.save_path = saveTorrentFile.toStdString();

    startFetchingMetadata(params);

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

void SaveTorrentDialog::setData(std::shared_ptr<const lt::torrent_info> ti)
{
    setDataFromTi(*ti);
}

void SaveTorrentDialog::on_changeSavePathButton_clicked()
{
    QString saveDir = QFileDialog::getExistingDirectory(this, tr("Choose a directory to save torrent in"));
    if (!saveDir.isEmpty()) {
        ui->savePathLineEdit->setText(saveDir);
    }
}

void SaveTorrentDialog::startFetchingMetadata(const lt::add_torrent_params& params)
{
    // Start fetching metadata
    m_fetcher = new MetadataFetcher{params};
    // TODO: Set not onyl size, but other data
    connect(m_fetcher, &MetadataFetcher::metadataFetched, this, &SaveTorrentDialog::setData);
    m_fetcher->start();

    // Simulating detached (hack)
    connect(m_fetcher, &QThread::finished, m_fetcher, &QThread::deleteLater);
    connect(m_fetcher, &MetadataFetcher::error, this, [this]{
        ui->sizeInfo->setText(tr("Failed"));
        ui->dateInfo->setText(tr("Failed"));
        QMessageBox::warning(this, tr("Warning"), tr("Metadata could not be fetched"));
    });
}

void SaveTorrentDialog::setDataFromTi(const libtorrent::torrent_info &ti)
{
    auto totalSize = ti.total_size();
    ui->sizeInfo->setText(utils::bytesToHigher(totalSize));

    auto bytesStr = utils::bytesToHigher(ti.total_size());
    ui->sizeInfo->setText(bytesStr);

    auto creationTime = ti.creation_date();
    if (creationTime == 0) {
        ui->dateInfo->setText("N/A");
    } else {
        auto tp = std::chrono::system_clock::from_time_t(creationTime);
        auto castTime = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch());
        auto datetime = QDateTime::fromSecsSinceEpoch(castTime.count());

        ui->dateInfo->setText(datetime.toString("MMM d HH:mm:ss yyyy"));
    }

    auto hashv1 = ti.info_hashes().get(lt::protocol_version::V1);
    auto hashv2 = ti.info_hashes().get(lt::protocol_version::V2);
    auto hashbest = ti.info_hashes().get_best();

    if (hashv1.is_all_zeros()) {
        ui->infoHashV1Value->setText("N/A");
    } else {
        ui->infoHashV1Value->setText(utils::toHex(ti.info_hashes().get(lt::protocol_version::V1).to_string()));
    }
    if (hashv2.is_all_zeros()) {
        ui->infoHashV2Value->setText("N/A");
    } else {
        ui->infoHashV2Value->setText(utils::toHex(ti.info_hashes().get(lt::protocol_version::V2).to_string()));
    }
    if (hashbest.is_all_zeros()) {
        ui->infoHashBestValue->setText("N/A");
    } else {
        ui->infoHashBestValue->setText(utils::toHex(ti.info_hashes().get_best().to_string()));
    }

    ui->commentValue->setText(QString::fromStdString(ti.comment()));
}

