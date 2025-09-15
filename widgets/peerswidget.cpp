#include "peerswidget.h"
#include "ui_peerswidget.h"
#include <QMenu>
#include <QAction>
#include <QPair>
#include "sessionmanager.h"
#include <QClipboard>
#include "addpeersdialog.h"

PeersWidget::PeersWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PeersWidget)
{
    ui->setupUi(this);

    ui->peerTable->setModel(&m_peerModel);
    ui->peerTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->peerTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->peerTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->peerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->peerTable, &QTableView::customContextMenuRequested, this, &PeersWidget::contextMenuRequested);
}

PeersWidget::~PeersWidget()
{
    delete ui;
}

void PeersWidget::clearPeers()
{
    m_peerModel.clearPeers();
}

void PeersWidget::contextMenuRequested(const QPoint &pos)
{
    auto index = ui->peerTable->indexAt(pos);

    QMenu menu(this);
    QAction* banAction = new QAction(tr("Ban peers"), this);
    connect(banAction, &QAction::triggered, this, [this] {
        const auto selectedRows = ui->peerTable->selectionModel()->selectedRows();
        QList<QPair<QString, unsigned short>> banPeers;
        for (const auto& modelIndex : selectedRows) {
            banPeers.append(m_peerModel.getPeerShortInfo(modelIndex.row()));
        }

        auto& sessionManager = SessionManager::instance();
        sessionManager.banPeers(banPeers);
    });

    QAction* copyAction = new QAction(tr("Copy ip:port"), this);
    connect(copyAction, &QAction::triggered, this, [this] {
        const auto selectedRows = ui->peerTable->selectionModel()->selectedRows();

        QString toCopyStr;
        for (const auto& modelIndex : selectedRows) {
            auto peer = m_peerModel.getPeerShortInfo(modelIndex.row());
            toCopyStr += peer.first + ":" + QString::number(peer.second) + '\n';
        }
        QApplication::clipboard()->setText(toCopyStr);
    });

    QAction* addPeerAction = new QAction(tr("Add a peer"), this);
    connect(addPeerAction, &QAction::triggered, this, [this] {
        AddPeersDialog dialog(this);

        if (dialog.exec() == QDialog::Accepted) {
            auto eps = dialog.getAddrs();
            auto& sessionManager = SessionManager::instance();
            auto currentTorrentId = sessionManager.getCurrentTorrentId();

            if (currentTorrentId.has_value()) {
                sessionManager.addPeersToTorrent(currentTorrentId.value(), eps);
            }
        }
    });

    if (index.row() == -1) {
        banAction->setEnabled(false);
        copyAction->setEnabled(false);
    }

    menu.addAction(banAction);
    menu.addAction(copyAction);
    menu.addAction(addPeerAction);

    menu.exec(ui->peerTable->viewport()->mapToGlobal(pos));
}

void PeersWidget::setPeers(const uint32_t id, const std::vector<libtorrent::peer_info> &peers)
{
    m_peerModel.setPeers(peers);
}
