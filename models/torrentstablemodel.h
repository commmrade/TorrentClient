#ifndef TORRENTSTABLEMODEL_H
#define TORRENTSTABLEMODEL_H
#include <QAbstractTableModel>


struct Torrent {
    std::uint32_t id;
    QString name;
    QString size;
    double progress; // 0.0% to 100.0%
    QString status;
    int seeds;
    int peers;
    QString downSpeed;
    QString upSpeed;
};

enum TorrentsFields {
    ID = 0,
    NAME,
    SIZE,
    PROGRESS,
    STATUS,
    SEEDS,
    PEERS,
    DOWN_SPEED,
    UP_SPEED
};
constexpr int TORRENT_FIELD_COUNT = TorrentsFields::UP_SPEED + 1;

class TorrentsTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TorrentsTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& index = QModelIndex{}) const override {
        return m_torrents.size();
    }
    int columnCount(const QModelIndex& index = QModelIndex{}) const override {
        return TORRENT_FIELD_COUNT;
    }

    QVariant data(const QModelIndex& index = QModelIndex{}, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index = QModelIndex{}, const QVariant& value = QVariant{}, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override {
        return QAbstractTableModel::flags(index);
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
private:
    QList<Torrent> m_torrents;
};

#endif // TORRENTSTABLEMODEL_H
