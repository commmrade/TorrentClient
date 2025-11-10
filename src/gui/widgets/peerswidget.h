#ifndef PEERSWIDGET_H
#define PEERSWIDGET_H

#include <QWidget>
#include "core/models/peertablemodel.h"
#include <libtorrent/peer_info.hpp>

namespace Ui
{
class PeersWidget;
}

class PeersWidget final : public QWidget
{
    Q_OBJECT

  public:
    explicit PeersWidget(QWidget *parent = nullptr);
    ~PeersWidget();
  public slots:
    void setPeers(const std::uint32_t id, const std::vector<lt::peer_info> &peers);
    void clearPeers();

  private:
    Ui::PeersWidget *ui;
    PeerTableModel   m_peerModel;

  private slots:
    void setupHeader();
    void headerMenuRequested(const QPoint &pos);

    void contextMenuRequested(const QPoint &pos);
};

#endif // PEERSWIDGET_H
