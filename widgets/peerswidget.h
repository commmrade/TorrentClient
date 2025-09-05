#ifndef PEERSWIDGET_H
#define PEERSWIDGET_H

#include <QWidget>
#include "peertablemodel.h"
#include <libtorrent/peer_info.hpp>

namespace Ui {
class PeersWidget;
}

class PeersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PeersWidget(QWidget *parent = nullptr);
    ~PeersWidget();
public slots:
    void setPeers(const std::uint32_t id, const std::vector<lt::peer_info>& peers);
    void clearPeers();

    void contextMenuRequested(const QPoint& pos);
private:
    Ui::PeersWidget *ui;
    PeerTableModel m_peerModel;
};

#endif // PEERSWIDGET_H
