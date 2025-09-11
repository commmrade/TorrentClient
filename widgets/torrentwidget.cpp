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

TorrentWidget::TorrentWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TorrentWidget)
    , m_sessionManager(SessionManager::instance())
    , m_speedGraph(new SpeedGraphWidget{})
{
    ui->setupUi(this);

    closeAllTabs();


    setupTableView();

    connect(&m_sessionManager, &SessionManager::torrentAdded, this, [this](const Torrent& torrent) {
        m_tableModel.addTorrent(torrent);
    });
    connect(&m_sessionManager, &SessionManager::torrentUpdated, this, [this](const Torrent& torrent) {
        m_tableModel.updateTorrent(torrent);
    });
    connect(&m_sessionManager, &SessionManager::torrentFinished, this, [this](const std::uint32_t id, const lt::torrent_status& status) {
        m_tableModel.finishTorrent(id, status);
    });
    connect(&m_sessionManager, &SessionManager::torrentDeleted, this, [this](const std::uint32_t id) {
        m_tableModel.removeTorrent(id);
    });

    connect(ui->tableView, &QTableView::clicked, this, [this](const QModelIndex& index) {
        auto torrentId = m_tableModel.getTorrentId(index.row());
        m_sessionManager.setCurrentTorrentId(torrentId);
    });

    // Context Menu Stuff
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &TorrentWidget::customContextMenu);

    m_sessionManager.loadResumes(); // Have to take care of resumes here, because otherwise i don't get torrentAdded signal
}

TorrentWidget::~TorrentWidget()
{
    delete ui;
}

void TorrentWidget::customContextMenu(const QPoint& pos)
{
    auto index = ui->tableView->indexAt(pos);

    if (index.row() == -1) return; /// indexAt() returns -1 when out of bounds
    auto torrentId = m_tableModel.getTorrentId(index.row());

    auto torrentStatus = m_tableModel.index(index.row(), getStatusIndex()).data().toString(); // Status was 4, now 3
    bool isPaused = m_sessionManager.isTorrentPaused(torrentId);
    QMenu menu(this);
    if (isPaused) {
        QAction* resumeAction = new QAction("Resume", this);
        connect(resumeAction, &QAction::triggered, this, [this, torrentId] {
            m_sessionManager.resumeTorrent(torrentId);
        });
        menu.addAction(resumeAction);
    } else {
        QAction* pauseAction = new QAction("Pause", this);
        connect(pauseAction, &QAction::triggered, this, [this, torrentId] {
            m_sessionManager.pauseTorrent(torrentId);
        });
        menu.addAction(pauseAction);
    }
    QAction* deleteAction = new QAction("Delete", this);
    connect(deleteAction, &QAction::triggered, this, [this, torrentId] {
        bool removeWithContents = true;

        DeleteTorrentDialog dialog{this};
        if (dialog.exec() == QDialog::Accepted) {
            if (!m_sessionManager.removeTorrent(torrentId, dialog.getRemoveWithContens())) {
                QMessageBox::warning(this, tr("Beware"), tr("Could not delete all torrent files, please finish it manually."));
            }
        }

    });
    menu.addAction(deleteAction);

    menu.exec(ui->tableView->viewport()->mapToGlobal(pos));
}

void TorrentWidget::setupTableView()
{
    ui->tableView->setModel(&m_tableModel);
    ui->tableView->setItemDelegateForColumn(2, &m_tableDelegate);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void TorrentWidget::on_pushButton_clicked()
{
    try {
        SaveTorrentDialog saveDialog{magnet_tag{}, ui->lineEdit->text(), this};
        if (saveDialog.exec() == QDialog::Accepted) {
            if (!m_sessionManager.addTorrentByMagnet(ui->lineEdit->text(), saveDialog.getSavePath())) {
                QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent"));
            }
        }
    } catch (const std::exception& ex) {
        qCritical() << ex.what();
        QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent: Invalid format"));
    }
}

void TorrentWidget::on_pushButton_2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open torrent", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), "Torrents (*.torrent)");
    if (filename.isEmpty()) {
        return;
    }
    try {
        SaveTorrentDialog saveDialog{torrent_file_tag{}, filename, this};
        if (saveDialog.exec() == QDialog::Accepted) {
            auto torrentSavePath = saveDialog.getSavePath();
            if (!m_sessionManager.addTorrentByFilename(filename, torrentSavePath)) {
                QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent"));
            }
        }
    } catch (const std::exception& ex) {
        qCritical() << ex.what();
        QMessageBox::critical(this, tr("Error"), tr("Could not add new torrent: Invalid format"));
    }
}

void TorrentWidget::on_togglePropertiesBtn_clicked()
{

    if (!ui->stackedWidget->isEnabled()) {
        ui->stackedWidget->setEnabled(true);
        ui->stackedWidget->setVisible(true);
        ui->stackedWidget->setCurrentIndex(0);
    } else {
        closeAllTabs();
    }
}


void TorrentWidget::on_toggleGraphsButton_clicked()
{
    if (!ui->stackedWidget->isEnabled()) {
        ui->stackedWidget->setEnabled(true);
        ui->stackedWidget->setVisible(true);
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        closeAllTabs();
    }
}

void TorrentWidget::closeAllTabs()
{
    ui->stackedWidget->setVisible(false);
    ui->stackedWidget->setEnabled(false);
}

