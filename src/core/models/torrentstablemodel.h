#ifndef TORRENTSTABLEMODEL_H
#define TORRENTSTABLEMODEL_H
#include <QAbstractTableModel>
#include "core/utils/torrent.h"

constexpr inline int getIdIndex() { return TORRENT_FIELD_COUNT + 1488; }

constexpr inline int getStatusIndex() { return STATUS; }

class TorrentsTableModel final : public QAbstractTableModel
{
    Q_OBJECT
  public:
    explicit TorrentsTableModel(QObject *parent = nullptr);

    int rowCount([[maybe_unused]] const QModelIndex &index = QModelIndex{}) const override
    {
        return m_torrents.size();
    }
    int columnCount([[maybe_unused]] const QModelIndex &index = QModelIndex{}) const override
    {
        // return TORRENT_FIELD_COUNT;
        return TORRENT_FIELD_COUNT - 2; // where -2 is remove id and category
    }

    QVariant data(const QModelIndex &index = QModelIndex{},
                  int                role  = Qt::DisplayRole) const override;
    bool     setData(const QModelIndex &index = QModelIndex{}, const QVariant &value = QVariant{},
                     int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return QAbstractTableModel::flags(index);
    }

    QVariant               headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addTorrent(const Torrent &torrent);
    bool updateTorrent(const Torrent &torrent);
    bool finishTorrent(const std::uint32_t id, const lt::torrent_status &status);
    bool removeTorrent(const std::uint32_t id);
    void setTorrentCategory(const std::uint32_t id, const QString &category);

    std::uint32_t getTorrentId(const int row) { return m_torrents[row].id; }

  private:
    QList<Torrent> m_torrents;
};

#endif // TORRENTSTABLEMODEL_H
