#include "propertieswidget.h"
#include "ui_propertieswidget.h"
#include <libtorrent/peer_info.hpp>

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PropertiesWidget)
{
    ui->setupUi(this);

    ui->propertiesTab->setTabText(0, "General");
    ui->propertiesTab->setTabText(1, "Trackers");
    ui->propertiesTab->setTabText(2, "Peers");
    ui->propertiesTab->setTabText(3, "HTTP Sources");

    ui->sourcesList->clear();
}

PropertiesWidget::~PropertiesWidget()
{
    delete ui;
}

void PropertiesWidget::setPeers(const std::uint32_t id, const std::vector<lt::peer_info>& peers)
{
    if (ui->propertiesTab->currentIndex() == 2 && isEnabled()) {
        ui->peerTable->setPeers(id, peers);
    }
}

void PropertiesWidget::clearPeers()
{
    if (ui->propertiesTab->currentIndex() == 2 && isEnabled()) {
        ui->peerTable->clearPeers();
    }
}

void PropertiesWidget::setGeneralInfo(const TorrentInfo &tInfo, const InternetInfo &iInfo)
{
    if (ui->propertiesTab->currentIndex() == 0 && isEnabled()) {
        ui->infoWidget->setGeneralInfo(tInfo, iInfo);
    }
}

void PropertiesWidget::clearGeneralInfo()
{
    if (ui->propertiesTab->currentIndex() == 0 && isEnabled()) {
        ui->infoWidget->clearGeneralInfo();
    }
}

void PropertiesWidget::setTrackers(const QList<Tracker> &trackers)
{
    qDebug() << ui->propertiesTab->currentIndex();
    if (ui->propertiesTab->currentIndex() == 1 && isEnabled()) {
        ui->trackersTable->setTrackers(trackers);
    }
}

void PropertiesWidget::clearTrackers()
{
    if (ui->propertiesTab->currentIndex() == 1 && isEnabled()) {
        ui->trackersTable->clearTrackers();
    }
}

void PropertiesWidget::setUrlSeeds(const std::set<std::string> &urlSeeds)
{
    qDebug() << "mty?" << urlSeeds.empty();
    if (ui->sourcesList->count() != 0) return;
    for (const auto& url : urlSeeds) {
        qDebug() << "aaass";
        ui->sourcesList->addItem(QString::fromStdString(url));
    }
}

void PropertiesWidget::clearUrlSeeds()
{
    ui->sourcesList->clear();
}
