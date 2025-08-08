#include "peerswidget.h"
#include "ui_peerswidget.h"

PeersWidget::PeersWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PeersWidget)
{
    ui->setupUi(this);


    ui->peerTable->setModel(&m_peerModel);
    ui->peerTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->peerTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->peerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

PeersWidget::~PeersWidget()
{
    delete ui;
}

void PeersWidget::clearPeers()
{
    m_peerModel.clearPeers();
}

void PeersWidget::setPeers(const uint32_t id, const std::vector<libtorrent::peer_info> &peers)
{
    m_peerModel.setPeers(peers);
}
