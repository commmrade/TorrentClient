#include "propertieswidget.h"
#include "ui_propertieswidget.h"
#include <libtorrent/peer_info.hpp>
#include "core/controllers/sessionmanager.h"

static constexpr int GENERAL_TAB_INDEX  = 0;
static constexpr int TRACKERS_TAB_INDEX = 1;
static constexpr int PEERS_TAB_INDEX    = 2;
static constexpr int SOURCES_TAB_INDEX  = 3;
static constexpr int FILES_TAB_INDEX    = 4;

PropertiesWidget::PropertiesWidget(QWidget *parent) : QWidget(parent), ui(new Ui::PropertiesWidget)
{
    ui->setupUi(this);
    ui->sourcesList->clear();

    setupPropertiesTabs();

    auto &sessionManager = SessionManager::instance();
    connect(&sessionManager, &SessionManager::peerInfo, this, &PropertiesWidget::setPeers);
    connect(&sessionManager, &SessionManager::clearPeerInfo, this, &PropertiesWidget::clearPeers);

    connect(&sessionManager, &SessionManager::generalInfo, this, &PropertiesWidget::setGeneralInfo);
    connect(&sessionManager, &SessionManager::clearGeneralInfo, this,
            &PropertiesWidget::clearGeneralInfo);

    connect(&sessionManager, &SessionManager::trackersInfo, this, &PropertiesWidget::setTrackers);
    connect(&sessionManager, &SessionManager::clearTrackers, this,
            &PropertiesWidget::clearTrackers);

    connect(&sessionManager, &SessionManager::urlSeedsInfo, this, &PropertiesWidget::setUrlSeeds);
    connect(&sessionManager, &SessionManager::clearUrlSeeds, this,
            &PropertiesWidget::clearUrlSeeds);

    connect(&sessionManager, &SessionManager::pieceBarInfo, this, &PropertiesWidget::setPieces);

    connect(&sessionManager, &SessionManager::filesInfo, this, &PropertiesWidget::setFiles);
    connect(&sessionManager, &SessionManager::clearFiles, this, &PropertiesWidget::clearFiles);
}

PropertiesWidget::~PropertiesWidget() { delete ui; }

void PropertiesWidget::setupPropertiesTabs()
{
    ui->propertiesTab->setCurrentIndex(0);
    ui->propertiesTab->setTabText(GENERAL_TAB_INDEX, tr("General"));
    ui->propertiesTab->setTabText(TRACKERS_TAB_INDEX, tr("Trackers"));
    ui->propertiesTab->setTabText(PEERS_TAB_INDEX, tr("Peers"));
    ui->propertiesTab->setTabText(SOURCES_TAB_INDEX, tr("HTTP Sources"));
    ui->propertiesTab->setTabText(FILES_TAB_INDEX, tr("Files"));
}

void PropertiesWidget::setPeers(const std::uint32_t id, const std::vector<lt::peer_info> &peers)
{
    if (ui->propertiesTab->currentIndex() == PEERS_TAB_INDEX && isEnabled())
    {
        ui->peerTable->setPeers(id, peers);
    }
}

void PropertiesWidget::clearPeers()
{
    if (ui->propertiesTab->currentIndex() == PEERS_TAB_INDEX && isEnabled())
    {
        ui->peerTable->clearPeers();
    }
}

void PropertiesWidget::setGeneralInfo(const TorrentInfo &tInfo, const InternetInfo &iInfo)
{
    if (ui->propertiesTab->currentIndex() == GENERAL_TAB_INDEX && isEnabled())
    {
        ui->infoWidget->setGeneralInfo(tInfo, iInfo);
    }
}

void PropertiesWidget::clearGeneralInfo()
{
    if (ui->propertiesTab->currentIndex() == GENERAL_TAB_INDEX && isEnabled())
    {
        ui->infoWidget->clearGeneralInfo();
    }
}

void PropertiesWidget::setTrackers(const QList<Tracker> &trackers)
{
    if (ui->propertiesTab->currentIndex() == TRACKERS_TAB_INDEX && isEnabled())
    {
        ui->trackersTable->setTrackers(trackers);
    }
}

void PropertiesWidget::clearTrackers()
{
    if (ui->propertiesTab->currentIndex() == TRACKERS_TAB_INDEX && isEnabled())
    {
        ui->trackersTable->clearTrackers();
    }
}

void PropertiesWidget::setPieces(const lt::typed_bitfield<libtorrent::piece_index_t> &pieces,
                                 const std::vector<int> &downloadingPiecesIndices)
{
    if (ui->propertiesTab->currentIndex() == GENERAL_TAB_INDEX && isEnabled())
    {
        ui->infoWidget->setPieces(pieces, downloadingPiecesIndices);
    }
}

void PropertiesWidget::setUrlSeeds(const std::set<std::string> &urlSeeds)
{
    // i think its pretty good for now, since url seeds dont change automatically
    if (ui->sourcesList->count() != 0)
        return;
    for (const auto &url : urlSeeds)
    {
        ui->sourcesList->addItem(QString::fromStdString(url));
    }
}

void PropertiesWidget::clearUrlSeeds() { ui->sourcesList->clear(); }

void PropertiesWidget::setFiles(const QList<File> &files)
{
    if (ui->propertiesTab->currentIndex() == FILES_TAB_INDEX && isEnabled())
    {
        ui->filesList->setFiles(files);
    }
}

void PropertiesWidget::clearFiles()
{
    if (ui->propertiesTab->currentIndex() == FILES_TAB_INDEX && isEnabled())
    {
        ui->filesList->clearFiles();
    }
}
