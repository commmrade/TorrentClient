#include "peerswidget.h"
#include "ui_peerswidget.h"
#include <QMenu>
#include <QAction>
#include <QPair>
#include "core/controllers/sessionmanager.h"
#include <QClipboard>
#include "gui/dialogs/addpeersdialog.h"
#include <QMessageBox>
#include <QSettings>
#include "core/utils/settingsvalues.h"

PeersWidget::PeersWidget(QWidget *parent) : QWidget(parent), ui(new Ui::PeersWidget)
{
    ui->setupUi(this);

    ui->peerTable->setModel(&m_peerModel);
    ui->peerTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->peerTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->peerTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->peerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->peerTable, &QTableView::customContextMenuRequested, this,
            &PeersWidget::contextMenuRequested);

    setupHeader();
}

PeersWidget::~PeersWidget() { delete ui; }

void PeersWidget::clearPeers() { m_peerModel.clearPeers(); }

void PeersWidget::setupHeader()
{
    QSettings    settings;
    QByteArray   headerState = settings.value(SettingsNames::DATA_PEERS_HEADER).toByteArray();
    QHeaderView *header      = ui->peerTable->horizontalHeader();
    if (!headerState.isEmpty())
    {
        header->restoreState(headerState);
    }
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header, &QHeaderView::customContextMenuRequested, this,
            &PeersWidget::headerMenuRequested);
}

void PeersWidget::headerMenuRequested(const QPoint &pos)
{
    auto *table     = ui->peerTable;
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
    }
    menu.exec(table->mapToGlobal(pos));

    if (isChanged)
    {
        QSettings  settings;
        QByteArray headerState = header->saveState();
        settings.setValue(SettingsNames::DATA_PEERS_HEADER, headerState);
    }
}

void PeersWidget::contextMenuRequested(const QPoint &pos)
{
    auto index = ui->peerTable->indexAt(pos);

    QMenu    menu(this);
    QAction *banAction = menu.addAction(tr("Ban peers"));
    connect(banAction, &QAction::triggered, this,
            [this]
            {
                const auto selectedRows = ui->peerTable->selectionModel()->selectedRows();
                QList<boost::asio::ip::address> banPeers;
                for (const auto &modelIndex : selectedRows)
                {
                    // banPeers.append(m_peerModel.getPeerShortInfo(modelIndex.row()));
                    auto ipStr = m_peerModel
                                     .data(m_peerModel.index(modelIndex.row(),
                                                             static_cast<int>(PeerFields::IP)))
                                     .toString();
                    auto addr = boost::asio::ip::make_address(ipStr.toStdString());
                    banPeers.append(std::move(addr));
                }

                auto &sessionManager = SessionManager::instance();
                sessionManager.banPeers(banPeers);
            });

    QAction *copyAction = menu.addAction(tr("Copy ip:port"));
    connect(copyAction, &QAction::triggered, this,
            [this]
            {
                const auto selectedRows = ui->peerTable->selectionModel()->selectedRows();

                QString toCopyStr;
                for (const auto &modelIndex : selectedRows)
                {
                    auto peer = m_peerModel.getPeerShortInfo(modelIndex.row());
                    toCopyStr += peer.first + ":" + QString::number(peer.second) + '\n';
                }
                QApplication::clipboard()->setText(toCopyStr);
            });

    QAction *addPeerAction = menu.addAction(tr("Add a peer"));
    connect(addPeerAction, &QAction::triggered, this,
            [this]
            {
                try
                {
                    AddPeersDialog dialog(this);

                    if (dialog.exec() == QDialog::Accepted)
                    {
                        auto  eps              = dialog.parseEndpoints();
                        auto &sessionManager   = SessionManager::instance();
                        auto  currentTorrentId = sessionManager.getCurrentTorrentId();

                        if (currentTorrentId.has_value())
                        {
                            sessionManager.addPeersToTorrent(currentTorrentId.value(), eps);
                        }
                    }
                }
                catch (const std::exception &ex)
                {
                    QMessageBox::warning(
                        this, tr("Warning"),
                        tr("Could not parse all addresses, make sure they are correct"));
                }
            });

    if (index.row() == -1)
    {
        banAction->setEnabled(false);
        copyAction->setEnabled(false);
    }

    menu.exec(ui->peerTable->viewport()->mapToGlobal(pos));
}

void PeersWidget::setPeers([[maybe_unused]] const uint32_t           id,
                           const std::vector<libtorrent::peer_info> &peers)
{
    m_peerModel.setPeers(peers);
}
