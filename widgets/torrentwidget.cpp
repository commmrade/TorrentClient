#include "torrentwidget.h"
#include "ui_torrentwidget.h"
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QMenu>
#include "savetorrentdialog.h"

TorrentWidget::TorrentWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TorrentWidget)
    , m_sessionManager(SessionManager::instance())
{
    ui->setupUi(this);

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

    // Context Menu Stuff
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &TorrentWidget::customContextMenu);
    m_sessionManager.loadResumes(); // Have to take care of resumes here, because otherwise i don't get torrentAdded signal
}

void TorrentWidget::customContextMenu(const QPoint& pos)
{
    auto index = ui->tableView->indexAt(pos);

    auto torrentId = m_tableModel.index(index.row(), 0).data().toUInt();
    auto torrentStatus = m_tableModel.index(index.row(), 4).data().toString();
    bool isPaused = m_sessionManager.isTorrentPaused(torrentId);
    qDebug() << "Paused" << isPaused;
    QMenu* menu = new QMenu(this);
    if (isPaused) {
        QAction* resumeAction = new QAction("Resume", this);
        connect(resumeAction, &QAction::triggered, this, [this, torrentId] {
            m_sessionManager.resumeTorrent(torrentId);
        });
        menu->addAction(resumeAction);
    } else {
        QAction* pauseAction = new QAction("Pause", this);
        connect(pauseAction, &QAction::triggered, this, [this, torrentId] {
            m_sessionManager.pauseTorrent(torrentId);
        });
        menu->addAction(pauseAction);
    }
    QAction* deleteAction = new QAction("Delete", this);
    connect(deleteAction, &QAction::triggered, this, [this, torrentId] {
        bool removeWithContents = true;
        m_sessionManager.removeTorrent(torrentId, removeWithContents);
    });
    menu->addAction(deleteAction);

    menu->popup(ui->tableView->viewport()->mapToGlobal(pos));
}


TorrentWidget::~TorrentWidget()
{
    delete ui;
}

void TorrentWidget::setupTableView()
{
    ui->tableView->setModel(&m_tableModel);
    ui->tableView->setItemDelegateForColumn(3, &m_tableDelegate);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void TorrentWidget::on_pushButton_clicked()
{
    SaveTorrentDialog saveDialog{0, ui->lineEdit->text()};
    if (saveDialog.exec() == QDialog::Accepted) {
        m_sessionManager.addTorrentByMagnet(ui->lineEdit->text(), saveDialog.getSavePath());
    }
}

void TorrentWidget::on_pushButton_2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "OPen torrent", "/home/klewy", "Torrents (*.torrent)");
    if (filename.isEmpty()) {
        return;
    }
    // TODO: Now show choose save location and etc dialog
    SaveTorrentDialog saveDialog{filename, this};
    if (saveDialog.exec() == QDialog::Accepted) {
        auto torrentSavePath = saveDialog.getSavePath();
        m_sessionManager.addTorrentByFilename(filename, torrentSavePath);
    }
}




