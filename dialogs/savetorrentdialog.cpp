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


    // Setup table
    setupTableView();
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

    setupTableView();
}

SaveTorrentDialog::~SaveTorrentDialog()
{
    delete ui;
}

void SaveTorrentDialog::setupTableView()
{
    ui->fileView->setModel(&m_fileModel);
    ui->fileView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->fileView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->fileView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->fileView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::STATUS), &m_statusDelegate);
    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::PROGRESS), &m_itemDelegate);
    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::PRIORITY), &m_priorityDelegate);


    ui->fileView->hideColumn(2);
    ui->fileView->hideColumn(3);
}

QString SaveTorrentDialog::getSavePath() const
{
    return ui->savePathLineEdit->text();
}

// void SaveTorrentDialog::setData(TorrentMetadata tmd)
// {
//     ui->sizeInfo->setText(utils::bytesToHigher(tmd.size));

//     auto creationTime = tmd.creationTime;
//     if (creationTime.isNull()) {
//         ui->dateInfo->setText("N/A");
//     } else {
//         ui->dateInfo->setText(creationTime.toString("MMM d HH:mm:ss yyyy"));
//     }

//     if (tmd.hashV1.isEmpty()) {
//         ui->infoHashV1Value->setText("N/A");
//     } else {
//         ui->infoHashV1Value->setText(tmd.hashV1);
//     }
//     if (tmd.hashV2.isEmpty()) {
//         ui->infoHashV2Value->setText("N/A");
//     } else {
//         ui->infoHashV2Value->setText(tmd.hashV2);
//     }
//     if (tmd.hashBest.isEmpty()) {
//         ui->infoHashBestValue->setText("N/A");
//     } else {
//         ui->infoHashBestValue->setText(tmd.hashBest);
//     }

//     ui->commentValue->setText(tmd.comment);
// }

void SaveTorrentDialog::setData(std::shared_ptr<const lt::torrent_info> ti) {
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
    auto* fetcher = new MetadataFetcher{params};
    auto* m_fetcherThread = new QThread{};
    fetcher->moveToThread(m_fetcherThread); // Doesn't block current thread

    // Start fetching when thead has started
    connect(m_fetcherThread, &QThread::started, fetcher, &MetadataFetcher::run);

    // Update data displayed in thi dialog
    connect(fetcher, &MetadataFetcher::metadataFetched, this, &SaveTorrentDialog::setData);

    // When fetching has been done, quit the thread and delete it and destroy the fetcher
    connect(fetcher, &MetadataFetcher::finished, fetcher, &QObject::deleteLater);
    connect(fetcher, &MetadataFetcher::finished, m_fetcherThread, &QThread::quit);
    connect(m_fetcherThread, &QThread::finished, m_fetcherThread, &QObject::deleteLater);

    // In case dialog is closed prematurily stop the fetcher, so the thread can exit
    connect(this, &QDialog::finished, fetcher, &MetadataFetcher::stopRunning);

    m_fetcherThread->start();
}


void SaveTorrentDialog::setDataFromTi(const libtorrent::torrent_info &ti)
{
    auto totalSize = ti.total_size();
    ui->sizeInfo->setText(utils::bytesToHigher(totalSize));

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


    // Set files

    const auto& tFiles = ti.files();

    QList<File> files;
    qDebug() << "There are" << tFiles.num_files() << "files";
    for (auto i = 0; i < tFiles.num_files(); ++i) {
        qDebug() << "FILENAME:" << tFiles.file_path(i) << "; SIZE:" << tFiles.file_size(i);
        File file;
        file.filename = QString::fromStdString(tFiles.file_path(i));
        file.isEnabled = true; // by default all are enabled
        file.filesize = tFiles.file_size(i);
        file.priority = lt::default_priority;

        files.append(std::move(file));
    }
    m_fileModel.setFiles(files);
}



