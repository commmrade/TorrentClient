#include "torrentwidget.h"
#include "ui_torrentwidget.h"
#include <QFileDialog>
#include <QTableWidgetItem>

TorrentWidget::TorrentWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TorrentWidget)
{
    ui->setupUi(this);

    setupTableView();

    connect(&m_sessionManager, &SessionManager::torrentAdded, this, [this](const Torrent& torrent) {
        qDebug() << "got EMIT";
        m_tableModel.addTorrent(torrent);
    });
    connect(&m_sessionManager, &SessionManager::torrentUpdated, this, [this](const Torrent& torrent) {
        m_tableModel.updateTorrent(torrent);
    });
    connect(&m_sessionManager, &SessionManager::torrentFinished, this, [this](const std::uint32_t id, const lt::torrent_status& status) {
        m_tableModel.finishTorrent(id, status);
    });

    m_sessionManager.loadResumes(); // Have to take care of resumes here, because otherwise i don't get torrentAdded signal
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
    m_sessionManager.addTorrentByMagnet(ui->lineEdit->text());
}

void TorrentWidget::on_pushButton_2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "OPen torrent", "/home/klewy", "Torrents (*.torrent)");
    if (!filename.isEmpty()) {
        m_sessionManager.addTorrentByFilename(filename);
    }
}



