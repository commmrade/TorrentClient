#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QMenu>
#include "gui/dialogs/savetorrentdialog.h"
#include <QElapsedTimer>
#include <QMessageBox>
#include "gui/widgets/speedgraphwidget.h"
#include "gui/dialogs/deletetorrentdialog.h"
#include "gui/dialogs/torrentsettingsdialog.h"
#include "QSettings"
#include "core/utils/settingsvalues.h"
#include <QSystemTrayIcon>
#include <QApplication>
#include "gui/dialogs/loadmagnetdialog.h"
#include "gui/dialogs/settingsdialog.h"
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_speedGraph(new SpeedGraphWidget)
    , m_sessionManager(SessionManager::instance())
    , m_categoryFilter(this)
{
    ui->setupUi(this);

    ui->stackedWidget->setEnabled(false);
    ui->stackedWidget->setVisible(false);

    setupTableView();
    setupTray();
    setupSession();
    setupTorrentHeader();

    connect(ui->torrentsView, &QTableView::clicked, this, &MainWindow::torrentClicked);
    ui->torrentsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->torrentsView, &QTableView::customContextMenuRequested, this,
            &MainWindow::customContextMenu);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::customContextMenu(const QPoint &pos)
{
    auto index = ui->torrentsView->indexAt(pos);

    if (index.row() == -1)
        return; /// indexAt() returns -1 when out of bounds
    auto torrentId = m_tableModel.getTorrentId(index.row());

    // auto torrentStatus =
    //     m_tableModel.index(index.row(), getStatusIndex()).data().toString(); // Status was 4, now
    //     3
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
                auto deleteTorrent = [this](const std::uint32_t torrentId, bool withContents)
                {
                    if (!m_sessionManager.removeTorrent(torrentId, withContents))
                    {
                        QMessageBox::warning(
                            this, tr("Beware"),
                            tr("Could not delete all torrent files, please finish it manually."));
                    }
                };

                QSettings settings;
                bool      confirmWhenDelete =
                    settings
                        .value(SettingsNames::TRANSFER_CONFIRM_DELETION,
                               SettingsValues::TRANSFER_CONFIRM_DELETION_DEFAULT)
                        .toBool();

                if (confirmWhenDelete)
                {
                    DeleteTorrentDialog dialog{this};
                    if (dialog.exec() == QDialog::Accepted)
                    {
                        deleteTorrent(torrentId, dialog.getRemoveWithContens());
                    }
                }
                else
                {
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
                connect(&settingsDialog, &TorrentSettingsDialog::maxNumOfConChanged, this,
                        [this, torrentId](int newValue)
                        { m_sessionManager.setTorrentMaxConn(torrentId, newValue); });
                connect(&settingsDialog, &TorrentSettingsDialog::dhtChanged, this,
                        [this, torrentId](bool disabled)
                        { m_sessionManager.setTorrentDht(torrentId, !disabled); });
                connect(&settingsDialog, &TorrentSettingsDialog::pexChanged, this,
                        [this, torrentId](bool disabled)
                        { m_sessionManager.setTorrentPex(torrentId, !disabled); });
                connect(&settingsDialog, &TorrentSettingsDialog::lsdChanged, this,
                        [this, torrentId](bool disabled)
                        { m_sessionManager.setTorrentLsd(torrentId, !disabled); });
                settingsDialog.exec();
            });
    menu.addAction(settingsAction);
    menu.exec(ui->torrentsView->viewport()->mapToGlobal(pos));
}

void MainWindow::setupTableView()
{
    m_categoryFilter.setSourceModel(&m_tableModel);
    ui->torrentsView->setModel(&m_categoryFilter);
    ui->torrentsView->setItemDelegateForColumn(2,
                                               &m_tableDelegate); // Delegate for QSlider progress

    ui->torrentsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->torrentsView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->torrentsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::setupTray()
{
    QSettings settings;
    bool      showTray =
        settings.value(SettingsNames::DESKTOP_SHOW_TRAY, SettingsValues::DESKTOP_SHOW_TRAY_DEFAULT)
            .toBool();
    if (!showTray)
        return;

    m_trayIcon = new QSystemTrayIcon{this};

    m_trayIcon->setIcon(QIcon{":/icon.png"});
    QMenu *trayMenu = new QMenu(this);
    toggleAction    = new QAction(tr("Hide"), this);
    connect(toggleAction, &QAction::triggered, this,
            [this]
            {
                if (isHidden())
                {
                    showNormal();
                    toggleAction->setText(tr("Hide"));
                }
                else
                {
                    hide();
                    toggleAction->setText(tr("Show"));
                }
            });
    trayMenu->addAction(toggleAction);

    QAction *addTorrentFilename = new QAction(tr("Add torrent file"), this);
    connect(addTorrentFilename, &QAction::triggered, this, [this] { on_pushButton_2_clicked(); });
    trayMenu->addAction(addTorrentFilename);

    QAction *addTorrentMagnet = new QAction(tr("Add torrent link"), this);
    connect(addTorrentMagnet, &QAction::triggered, this,
            [this]
            {
                LoadMagnetDialog magnetDialog{this};
                if (magnetDialog.exec() == QDialog::Accepted)
                {
                    QList<QString> magnets = magnetDialog.getMagnets();
                    for (const auto &magnet : std::as_const(magnets))
                    {
                        addTorrentByMagnet(magnet);
                    }
                }
            });
    trayMenu->addAction(addTorrentMagnet);

    QAction *exit = new QAction(tr("Exit"), this);
    connect(exit, &QAction::triggered, this, [] { QApplication::exit(); });
    trayMenu->addAction(exit);
    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog dialog(this);
    dialog.exec(); // idc if it is accepted or rejected
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    bool      showTray =
        settings.value(SettingsNames::DESKTOP_SHOW_TRAY, SettingsValues::DESKTOP_SHOW_TRAY_DEFAULT)
            .toBool();
    int exitBehaviour =
        settings.value(SettingsNames::DESKTOP_EXIT_BEH, SettingsValues::DESKTOP_EXIT_BEH_CLOSE)
            .toInt();
    if (exitBehaviour == SettingsValues::DESKTOP_EXIT_BEH_CLOSE || !showTray)
    {
        QMainWindow::closeEvent(event);
    }
    else
    {
        hide();
        toggleAction->setText(tr("Show"));
        event->ignore();
    }
}

void MainWindow::headerContextMenu(const QPoint &pos)
{
    auto *table     = ui->torrentsView;
    auto *header    = table->horizontalHeader();
    bool  isChanged = false;

    QMenu menu(this);
    for (auto i = 0; i < header->count(); ++i)
    {
        QString headerName = table->model()->headerData(i, Qt::Horizontal).toString();
        bool    isHidden   = header->isSectionHidden(i);

        QAction *action = menu.addAction(headerName);
        action->setCheckable(true);
        action->setChecked(!isHidden);

        connect(action, &QAction::triggered, this,
                [header, i, isHidden, &isChanged]() mutable
                {
                    header->setSectionHidden(i, !isHidden);
                    isChanged = true;
                });
        menu.addAction(action);
    }

    menu.exec(table->mapToGlobal(pos));
    if (isChanged)
    {
        QSettings  settings;
        QByteArray headerState = header->saveState();
        settings.setValue(SettingsNames::DATA_TORRENTS_HEADER, headerState);
    }
}

void MainWindow::showMessage(QStringView msg)
{
    if (m_trayIcon)
    {
        QSettings settings;
        bool      notifsEnabled = settings
                                 .value(SettingsNames::DESKTOP_SHOW_NOTIFS,
                                        SettingsValues::DESKTOP_SHOW_NOTIFS_DEFAULT)
                                 .toBool();
        if (notifsEnabled)
            m_trayIcon->showMessage(tr("Notification"), QString{msg});
    }
}

void MainWindow::setupSession()
{
    connect(&m_sessionManager, &SessionManager::torrentAdded, this,
            [this](const Torrent &torrent) { m_tableModel.addTorrent(torrent); });
    connect(&m_sessionManager, &SessionManager::torrentUpdated, this,
            [this](const Torrent &torrent) { m_tableModel.updateTorrent(torrent); });
    connect(&m_sessionManager, &SessionManager::torrentFinished, this,
            [this](const std::uint32_t id, const lt::torrent_status &status)
            {
                m_tableModel.finishTorrent(id, status);
                showMessage(QString{tr("Torrent '%1' has been finished")}.arg(status.name));
            });
    connect(&m_sessionManager, &SessionManager::torrentDeleted, this,
            [this](const std::uint32_t id) { m_tableModel.removeTorrent(id); });
    connect(&m_sessionManager, &SessionManager::torrentFileMoveFailed, this,
            [this](const QString &msg, const QString &torrentName)
            {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Could not move a file, torrent:") + torrentName +
                                          tr(", because ") + msg);
            });
    m_sessionManager.loadResumes(); // Have to take care of resumes here, because otherwise i don't
                                    // get torrentAdded signal
}

void MainWindow::setupTorrentHeader()
{
    QSettings    settings;
    QByteArray   headerState = settings.value(SettingsNames::DATA_TORRENTS_HEADER).toByteArray();
    QHeaderView *header      = ui->torrentsView->horizontalHeader();
    if (!headerState.isEmpty())
    {
        header->restoreState(headerState);
    }

    header->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header, &QHeaderView::customContextMenuRequested, this, &MainWindow::headerContextMenu);
}

void MainWindow::addTorrentByMagnet(const QString &magnetUri)
{
    try
    {
        SaveTorrentDialog *saveDialog = new SaveTorrentDialog{magnet_tag{}, magnetUri};

        connect(
            saveDialog, &SaveTorrentDialog::accepted, this,
            [saveDialog, magnetUri, this]()
            {
                auto torrentInfo    = saveDialog->getTorrentInfo();
                auto filePriorities = saveDialog->getFilePriorities();
                auto savePath       = saveDialog->getSavePath();

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
        connect(saveDialog, &SaveTorrentDialog::finished, saveDialog,
                &SaveTorrentDialog::deleteLater);
        saveDialog->open();
    }
    catch (const std::exception &ex)
    {
        qCritical() << ex.what();
        QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent: Invalid format"));
    }
}

void MainWindow::addTorrentByFile(const QString &filepath)
{
    try
    {
        SaveTorrentDialog saveDialog{torrent_file_tag{}, filepath, this};
        if (saveDialog.exec() == QDialog::Accepted)
        {
            auto savePath       = saveDialog.getSavePath();
            auto torrentInfo    = saveDialog.getTorrentInfo();
            auto filePriorities = saveDialog.getFilePriorities();
            if (!m_sessionManager.addTorrentByTorrentInfo(torrentInfo, filePriorities, savePath))
            {
                QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent"));
            }
        }
    }
    catch (const std::exception &ex)
    {
        qCritical() << ex.what();
        QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent: Invalid format"));
    }
}

void MainWindow::on_pushButton_clicked()
{
    LoadMagnetDialog magnetDialog{this};
    if (magnetDialog.exec() == QDialog::Accepted)
    {
        QList<QString> magnets = magnetDialog.getMagnets();
        for (const auto &magnet : std::as_const(magnets))
        {
            addTorrentByMagnet(magnet);
        }
    }
}

void MainWindow::on_pushButton_2_clicked()
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

void MainWindow::on_togglePropertiesBtn_clicked()
{
    switchTab(0);
}

void MainWindow::on_toggleGraphsButton_clicked()
{
    switchTab(1);
}

void MainWindow::switchTab(int index) {
    if (ui->stackedWidget->currentIndex() == index) {
        ui->stackedWidget->setEnabled(!ui->stackedWidget->isEnabled());
        ui->stackedWidget->setVisible(!ui->stackedWidget->isVisible());
    } else {
        ui->stackedWidget->setCurrentIndex(index);
        ui->stackedWidget->setEnabled(true);
        ui->stackedWidget->setVisible(true);
    }
}

void MainWindow::on_categoriesList_currentTextChanged(const QString &currentText)
{
    m_categoryFilter.setCategory(currentText);
    m_categoryFilter.invalidate();
}

void MainWindow::torrentClicked(const QModelIndex &index)
{
    auto torrentId = m_tableModel.getTorrentId(index.row());
    m_sessionManager.setCurrentTorrentId(torrentId);
    m_sessionManager.forceUpdateProperties();
}
