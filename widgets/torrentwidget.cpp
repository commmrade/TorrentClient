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
#include "QSettings"
#include "settingsvalues.h"
#include <QSystemTrayIcon>
#include <QApplication>
#include "loadmagnetdialog.h"

TorrentWidget::TorrentWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TorrentWidget)
    , m_sessionManager(SessionManager::instance())
    , m_speedGraph(new SpeedGraphWidget{})
    , m_categoryFilter(this)
    , m_trayIcon(new QSystemTrayIcon{this})
{
    ui->setupUi(this);

    closeAllTabs();

    setupTableView();
    setupTray();
    setupSession();

    connect(ui->torrentsView, &QTableView::clicked, this, &TorrentWidget::torrentClicked);
    ui->torrentsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->torrentsView, &QTableView::customContextMenuRequested, this,
            &TorrentWidget::customContextMenu);
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
                auto deleteTorrent = [this](const std::uint32_t torrentId, bool withContents) {
                    if (!m_sessionManager.removeTorrent(torrentId, withContents))
                    {
                        QMessageBox::warning(
                            this, tr("Beware"),
                            tr("Could not delete all torrent files, please finish it manually."));
                    }
                };

                QSettings settings;
                bool confirmWhenDelete = settings.value(SettingsValues::TRANSFER_CONFIRM_DELETION, true).toBool();

                if (confirmWhenDelete) {
                    DeleteTorrentDialog dialog{this};
                    if (dialog.exec() == QDialog::Accepted)
                    {
                        deleteTorrent(torrentId, dialog.getRemoveWithContens());
                    }
                } else {
                    deleteTorrent(torrentId, true);
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

void TorrentWidget::setupTray()
{
    m_trayIcon->setIcon(QIcon{"icon.png"});
    QMenu* trayMenu = new QMenu(this);
    QAction* toggleMenu = new QAction(tr("Hide"), this);
    connect(toggleMenu, &QAction::triggered, this, []{
        qDebug() << "Hide action";
        // TODO: Daemonization
    });
    trayMenu->addAction(toggleMenu);

    QAction* addTorrentFilename = new QAction(tr("Add torrent file"), this);
    connect(addTorrentFilename, &QAction::triggered, this, [this] {
        on_pushButton_2_clicked();
    });
    trayMenu->addAction(addTorrentFilename);

    QAction* addTorrentMagnet = new QAction(tr("Add torrent link"), this);
    connect(addTorrentMagnet, &QAction::triggered, this, [this] {
        LoadMagnetDialog magnetDialog{this};
        if (magnetDialog.exec() == QDialog::Accepted) {
            QList<QString> magnets = magnetDialog.getMagnets();
            // TODO: Open all save torrent dialogs at once somehow
            for (const auto& magnet : std::as_const(magnets)) {
                addTorrentByMagnet(magnet);
            }
        }
    });
    trayMenu->addAction(addTorrentMagnet);

    QAction* exit = new QAction(tr("Exit"), this);
    connect(exit, &QAction::triggered, this, []{
        QApplication::exit();
    });
    trayMenu->addAction(exit);
    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();
}

void TorrentWidget::setupSession()
{
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
    m_sessionManager.loadResumes(); // Have to take care of resumes here, because otherwise i don't
                                    // get torrentAdded signal

}

void TorrentWidget::addTorrentByMagnet(const QString &magnetUri)
{
    try
    {
        SaveTorrentDialog saveDialog{magnet_tag{}, magnetUri, this};
        // TODO: Why the fuck do i even use a signal here, i can just return what i need in get method
        connect(
            &saveDialog, &SaveTorrentDialog::torrentConfirmed, this,
            [this, &saveDialog, &magnetUri](std::shared_ptr<const lt::torrent_info> torrentInfo,
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
                    if (!m_sessionManager.addTorrentByMagnet(magnetUri, savePath))
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

void TorrentWidget::addTorrentByFile(const QString &filepath)
{
    try
    {
        SaveTorrentDialog saveDialog{torrent_file_tag{}, filepath, this};
        // TODO: Why the fuck do i even use a signal here, i can just return what i need in get method
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

void TorrentWidget::on_pushButton_clicked()
{
    LoadMagnetDialog magnetDialog{this};
    if (magnetDialog.exec() == QDialog::Accepted) {
        QList<QString> magnets = magnetDialog.getMagnets();
        // TODO: Open all save torrent dialogs at once somehow
        // maybe i can just create all of them on the heap and .show() and then connect closed to delete later
        for (const auto& magnet : std::as_const(magnets)) {
            addTorrentByMagnet(magnet);
        }
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
    addTorrentByFile(filename);
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

void TorrentWidget::torrentClicked(const QModelIndex &index)
{
    auto torrentId = m_tableModel.getTorrentId(index.row());
    m_sessionManager.setCurrentTorrentId(torrentId);
    m_sessionManager.forceUpdateProperties();
}
