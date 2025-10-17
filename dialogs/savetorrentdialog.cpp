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

SaveTorrentDialog::SaveTorrentDialog(torrent_file_tag, const QString &torrentPath, QWidget *parent)
    : QDialog(parent), ui(new Ui::SaveTorrentDialog)
{
    ui->setupUi(this);
    // Setup table
    init();

    auto info     = std::make_shared<lt::torrent_info>(torrentPath.toStdString());
    m_torrentInfo = std::move(info);
    setDataFromTi();

    QSettings settings;
    auto      savePath = settings
                        .value(SettingsNames::SESSION_DEFAULT_SAVE_LOCATION,
                               QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))
                        .toString();
    ui->savePathLineEdit->setText(savePath);
}

SaveTorrentDialog::SaveTorrentDialog(magnet_tag, const QString &magnetUri, QWidget *parent)
    : QDialog(parent), ui(new Ui::SaveTorrentDialog)
{
    ui->setupUi(this);
    init();
    ui->sizeInfo->setText(tr("Fetching..."));

    lt::add_torrent_params params = lt::parse_magnet_uri(magnetUri.toStdString());
    auto saveTorrentFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                           QDir::separator() + Dirs::TORRENTS + QDir::separator() +
                           utils::toHex(params.info_hashes.get_best().to_string());
    params.save_path = saveTorrentFile.toStdString();
    startFetchingMetadata(params);

    QSettings settings;
    auto      savePath = settings
                        .value(SettingsNames::SESSION_DEFAULT_SAVE_LOCATION,
                               QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))
                        .toString();
    ui->savePathLineEdit->setText(savePath);
}

SaveTorrentDialog::~SaveTorrentDialog() { delete ui; }

void SaveTorrentDialog::setupTableView()
{
    ui->fileView->setModel(&m_fileModel);
    ui->fileView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->fileView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->fileView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->fileView->header()->setSectionResizeMode(ui->fileView->header()->count() - 1,
                                                 QHeaderView::Stretch);
    ui->fileView->setAnimated(true);

    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::STATUS), &m_statusDelegate);
    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::PROGRESS), &m_itemDelegate);
    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::PRIORITY),
                                           &m_priorityDelegate);

    ui->fileView->hideColumn(2);
    ui->fileView->hideColumn(3);

    // TODO: connect signals from file model to updatge status and priority for files
    connect(&m_fileModel, &FileTreeModel::statusChanged, this,
            [this](int index, bool value)
            {
                m_filePriorities[index] = value ? lt::default_priority : lt::dont_download;

                // Recalc total size TODO:
            });
    connect(&m_fileModel, &FileTreeModel::priorityChanged, this,
            [this](int index, int priority)
            {
                m_filePriorities[index] = priority;
                // Recalc total size TODO:
            });
}

void SaveTorrentDialog::init()
{
    setupTableView();
}

QString SaveTorrentDialog::getSavePath() const { return ui->savePathLineEdit->text(); }

QList<libtorrent::download_priority_t> SaveTorrentDialog::getFilePriorities() const
{
    return m_filePriorities;
}

std::shared_ptr<const libtorrent::torrent_info> SaveTorrentDialog::getTorrentInfo() const
{
    return m_torrentInfo;
}

void SaveTorrentDialog::setData(std::shared_ptr<const lt::torrent_info> ti)
{
    m_torrentInfo = std::move(ti);
    setDataFromTi();
}

void SaveTorrentDialog::on_changeSavePathButton_clicked()
{
    QString saveDir =
        QFileDialog::getExistingDirectory(this, tr("Choose a directory to save torrent in"));
    if (!saveDir.isEmpty())
    {
        ui->savePathLineEdit->setText(saveDir);
    }
}

void SaveTorrentDialog::startFetchingMetadata(const lt::add_torrent_params &params)
{
    auto *fetcher         = new MetadataFetcher{params};
    auto *m_fetcherThread = new QThread{};
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

void SaveTorrentDialog::setDataFromTi()
{
    auto totalSize = m_torrentInfo->total_size();
    ui->sizeInfo->setText(utils::bytesToHigher(totalSize));

    auto creationTime = m_torrentInfo->creation_date();
    if (creationTime == 0)
    {
        ui->dateInfo->setText("N/A");
    }
    else
    {
        auto tp       = std::chrono::system_clock::from_time_t(creationTime);
        auto castTime = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch());
        auto datetime = QDateTime::fromSecsSinceEpoch(castTime.count());

        ui->dateInfo->setText(datetime.toString("MMM d HH:mm:ss yyyy"));
    }

    auto hashv1   = m_torrentInfo->info_hashes().get(lt::protocol_version::V1);
    auto hashv2   = m_torrentInfo->info_hashes().get(lt::protocol_version::V2);
    auto hashbest = m_torrentInfo->info_hashes().get_best();

    if (hashv1.is_all_zeros())
    {
        ui->infoHashV1Value->setText("N/A");
    }
    else
    {
        ui->infoHashV1Value->setText(
            utils::toHex(m_torrentInfo->info_hashes().get(lt::protocol_version::V1).to_string()));
    }
    if (hashv2.is_all_zeros())
    {
        ui->infoHashV2Value->setText("N/A");
    }
    else
    {
        ui->infoHashV2Value->setText(
            utils::toHex(m_torrentInfo->info_hashes().get(lt::protocol_version::V2).to_string()));
    }
    if (hashbest.is_all_zeros())
    {
        ui->infoHashBestValue->setText("N/A");
    }
    else
    {
        ui->infoHashBestValue->setText(
            utils::toHex(m_torrentInfo->info_hashes().get_best().to_string()));
    }

    ui->commentValue->setText(QString::fromStdString(m_torrentInfo->comment()));

    // Set files

    const auto &tFiles   = m_torrentInfo->files();
    auto        numFiles = tFiles.num_files();
    QList<File> files;

    // Track file priorities
    m_filePriorities.resize(numFiles);
    for (auto i = 0; i < numFiles; ++i)
    {
        // qDebug() << "FILENAME:" << tFiles.file_path(i) << "; SIZE:" << tFiles.file_size(i);
        File file;
        file.id             = i;
        file.filename       = QString::fromStdString(tFiles.file_path(i));
        file.isEnabled      = true; // by default all are enabled
        file.filesize       = tFiles.file_size(i);
        file.priority       = lt::default_priority;
        m_filePriorities[i] = lt::default_priority;

        files.append(std::move(file));
    }
    m_fileModel.setFiles(files);
}


