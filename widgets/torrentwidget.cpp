#include "torrentwidget.h"
#include "ui_torrentwidget.h"
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QMenu>
#include "savetorrentdialog.h"
#include <QElapsedTimer>
#include <QMessageBox>
#include "speedgraphwidget.h"
#include "deletetorrentdialog.h"
#include "torrentsettingsdialog.h"

TorrentWidget::TorrentWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TorrentWidget)
    , m_sessionManager(SessionManager::instance())
    , m_speedGraph(new SpeedGraphWidget{})
    , m_categoryFilter(this)
{
    ui->setupUi(this);

    closeAllTabs();

    setupTableView();

    connect(&m_sessionManager, &SessionManager::torrentAdded, this,
            [this](const Torrent &torrent) { m_tableModel.addTorrent(torrent); });
    connect(&m_sessionManager, &SessionManager::torrentUpdated, this,
            [this](const Torrent &torrent) { m_tableModel.updateTorrent(torrent); });
    connect(&m_sessionManager, &SessionManager::torrentFinished, this,
            [this](const std::uint32_t id, const lt::torrent_status &status)
            { m_tableModel.finishTorrent(id, status); });
    connect(&m_sessionManager, &SessionManager::torrentDeleted, this,
            [this](const std::uint32_t id) { m_tableModel.removeTorrent(id); });
    connect(&m_sessionManager, &SessionManager::torrentFileMoveFailed, this,
            [this](const QString &msg, const QString &torrentName)
            {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Could not move a file, torrent:") + torrentName + tr(", because ") + msg);
            });

    connect(ui->torrentsView, &QTableView::clicked, this,
            [this](const QModelIndex &index)
            {
                auto torrentId = m_tableModel.getTorrentId(index.row());
                m_sessionManager.setCurrentTorrentId(torrentId);
                m_sessionManager.forceUpdateProperties();
            });

    // Context Menu Stuff
    ui->torrentsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->torrentsView, &QTableView::customContextMenuRequested, this,
            &TorrentWidget::customContextMenu);

    m_sessionManager.loadResumes(); // Have to take care of resumes here, because otherwise i don't
                                    // get torrentAdded signal
}

TorrentWidget::~TorrentWidget() { delete ui; }

void TorrentWidget::customContextMenu(const QPoint &pos)
{
    auto index = ui->torrentsView->indexAt(pos);

    if (index.row() == -1)
        return; /// indexAt() returns -1 when out of bounds
    auto torrentId = m_tableModel.getTorrentId(index.row());

    auto torrentStatus =
        m_tableModel.index(index.row(), getStatusIndex()).data().toString(); // Status was 4, now 3
    bool  isPaused = m_sessionManager.isTorrentPaused(torrentId);
    QMenu menu(this);
    if (isPaused)
    {
        QAction *resumeAction = new QAction(tr("Resume"), this);
        connect(resumeAction, &QAction::triggered, this,
                [this, torrentId] { m_sessionManager.resumeTorrent(torrentId); });
        menu.addAction(resumeAction);
    }
    else
    {
        QAction *pauseAction = new QAction(tr("Pause"), this);
        connect(pauseAction, &QAction::triggered, this,
                [this, torrentId] { m_sessionManager.pauseTorrent(torrentId); });
        menu.addAction(pauseAction);
    }
    QAction *deleteAction = new QAction(tr("Delete"), this);
    connect(deleteAction, &QAction::triggered, this,
            [this, torrentId]
            {
                bool removeWithContents = true;

                DeleteTorrentDialog dialog{this};
                if (dialog.exec() == QDialog::Accepted)
                {
                    if (!m_sessionManager.removeTorrent(torrentId, dialog.getRemoveWithContens()))
                    {
                        QMessageBox::warning(
                            this, tr("Beware"),
                            tr("Could not delete all torrent files, please finish it manually."));
                    }
                }
            });
    menu.addAction(deleteAction);
    QAction *settingsAction = new QAction(tr("Settings"), this);
    connect(settingsAction, &QAction::triggered, this,
            [this, torrentId]
            {
                const TorrentHandle   tHandle = m_sessionManager.getTorrentHandle(torrentId);
                TorrentSettingsDialog settingsDialog(tHandle, this);

                // These only fire when dialog is accepted
                connect(&settingsDialog, &TorrentSettingsDialog::downloadLimitChanged, this,
                        [this, torrentId](int newLimit)
                        { m_sessionManager.setTorrentDownloadLimit(torrentId, newLimit); });
                connect(&settingsDialog, &TorrentSettingsDialog::uploadLimitChanged, this,
                        [this, torrentId](int newLimit)
                        { m_sessionManager.setTorrentUploadLimit(torrentId, newLimit); });
                connect(&settingsDialog, &TorrentSettingsDialog::savePathChanged, this,
                        [this, torrentId](const QString &newPath)
                        { m_sessionManager.setTorrentSavePath(torrentId, newPath); });
                settingsDialog.exec();
            });
    menu.addAction(settingsAction);

    menu.exec(ui->torrentsView->viewport()->mapToGlobal(pos));
}

void TorrentWidget::setupTableView()
{
    m_categoryFilter.setSourceModel(&m_tableModel);
    ui->torrentsView->setModel(&m_categoryFilter);
    ui->torrentsView->setItemDelegateForColumn(2,
                                               &m_tableDelegate); // Delegate for QSlider progress

    ui->torrentsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->torrentsView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->torrentsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void TorrentWidget::on_pushButton_clicked()
{
    try
    {
        SaveTorrentDialog saveDialog{magnet_tag{}, ui->lineEdit->text(), this};
        connect(
            &saveDialog, &SaveTorrentDialog::torrentConfirmed, this,
            [this, &saveDialog](std::shared_ptr<const lt::torrent_info> torrentInfo,
                                QList<lt::download_priority_t>          filePriorities)
            {
                auto savePath = saveDialog.getSavePath();
                if (torrentInfo)
                {
                    if (!m_sessionManager.addTorrentByTorrentInfo(torrentInfo, filePriorities,
                                                                  savePath))
                    {
                        QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent"));
                    }
                }
                else
                {
                    if (!m_sessionManager.addTorrentByMagnet(ui->lineEdit->text(), savePath))
                    {
                        QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent"));
                    }
                }
            });
        saveDialog.exec();
    }
    catch (const std::exception &ex)
    {
        qCritical() << ex.what();
        QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent: Invalid format"));
    }
}

void TorrentWidget::on_pushButton_2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open torrent"),
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), "Torrents (*.torrent)");
    if (filename.isEmpty())
    {
        return;
    }
    try
    {
        SaveTorrentDialog saveDialog{torrent_file_tag{}, filename, this};
        connect(&saveDialog, &SaveTorrentDialog::torrentConfirmed, this,
                [this, &saveDialog](std::shared_ptr<const lt::torrent_info> torrentInfo,
                                    QList<lt::download_priority_t>          filePriorities)
                {
                    auto savePath = saveDialog.getSavePath();
                    if (!m_sessionManager.addTorrentByTorrentInfo(torrentInfo, filePriorities,
                                                                  savePath))
                    {
                        QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent"));
                    }
                });
        saveDialog.exec();
    }
    catch (const std::exception &ex)
    {
        qCritical() << ex.what();
        QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent: Invalid format"));
    }
}

void TorrentWidget::on_togglePropertiesBtn_clicked()
{
    if (!ui->stackedWidget->isEnabled())
    {
        ui->stackedWidget->setEnabled(true);
        ui->stackedWidget->setVisible(true);
        ui->stackedWidget->setCurrentIndex(0);
    }
    else
    {
        closeAllTabs();
    }
}

void TorrentWidget::on_toggleGraphsButton_clicked()
{
    if (!ui->stackedWidget->isEnabled())
    {
        ui->stackedWidget->setEnabled(true);
        ui->stackedWidget->setVisible(true);
        ui->stackedWidget->setCurrentIndex(1);
    }
    else
    {
        closeAllTabs();
    }
}

void TorrentWidget::closeAllTabs()
{
    ui->stackedWidget->setVisible(false);
    ui->stackedWidget->setEnabled(false);
}

void TorrentWidget::on_categoriesList_currentTextChanged(const QString &currentText)
{
    m_categoryFilter.setCategory(currentText);
    m_categoryFilter.invalidate();
}
