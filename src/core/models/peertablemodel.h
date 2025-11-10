#ifndef PEERTABLEMODEL_H
#define PEERTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include "core/utils/peer.h"
#include <libtorrent/peer_info.hpp>
#include <maxminddb.h>

class PeerTableModel final : public QAbstractTableModel
{
    Q_OBJECT
  public:
    explicit PeerTableModel(QObject *parent = nullptr);
    ~PeerTableModel();

    int rowCount([[maybe_unused]] const QModelIndex &index = QModelIndex{}) const override
    {
        return m_peers.size();
    }
    int columnCount([[maybe_unused]] const QModelIndex &index = QModelIndex{}) const override
    {
        return PEER_FIELD_COUNT;
    }

    QVariant data(const QModelIndex &index = QModelIndex{},
                  int                role  = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void                               setPeers(const std::vector<lt::peer_info> &peers);
    void                               clearPeers();
    std::pair<QString, unsigned short> getPeerShortInfo(int index);
  signals:

  private:
    QList<Peer> m_peers;
    MMDB_s      m_mmdb;

    QString countryFromIp(QByteArrayView ip);
};

#endif // PEERTABLEMODEL_H
