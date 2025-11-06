#ifndef TRACKERTABLEMODEL_H
#define TRACKERTABLEMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include "core/utils/tracker.h"

class TrackerTableModel : public QAbstractTableModel
{
    Q_OBJECT
  public:
    explicit TrackerTableModel(QObject *parent = nullptr);

    int rowCount([[maybe_unused]] const QModelIndex &index = QModelIndex{}) const override
    {
        return m_trackers.size();
    }
    int columnCount([[maybe_unused]] const QModelIndex &index = QModelIndex{}) const override
    {
        return TRACKER_FIELDS_COUNT;
    }

    QVariant data(const QModelIndex &index = QModelIndex{},
                  int                role  = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTrackers(const QList<Tracker> &trackers);
    void clearTrackers();
  signals:

  private:
    QList<Tracker> m_trackers;
};

#endif // TRACKERTABLEMODEL_H
