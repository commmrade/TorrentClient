#include "trackertablemodel.h"

TrackerTableModel::TrackerTableModel(QObject *parent) : QAbstractTableModel(parent) {}

QVariant TrackerTableModel::data(const QModelIndex &index, int role) const
{
    if (role < Qt::UserRole) {
        if (role == Qt::DisplayRole) {
            auto& tracker = m_trackers[index.row()];
            switch ((TrackerFields)index.column()) {
                case TrackerFields::URL: {
                    return QVariant{tracker.url};
                }
                case TrackerFields::TIER: {
                    return QVariant{tracker.tier};
                }
                case TrackerFields::STATUS: {
                    QString r = tracker.isWorking ? "Working" : "Stopped";
                    return QVariant{r};
                }
                case TrackerFields::SEEDS: {
                    return QVariant{tracker.seeds};
                }
                case TrackerFields::LEECHES: {
                    return QVariant{tracker.leeches};
                }
                case TrackerFields::MESSAGE: {
                    return QVariant{tracker.message};
                }
                case TrackerFields::NEXT_ANNOUNCE: {
                    return QVariant{tracker.nextAnnounce};
                }
                case TrackerFields::MIN_ANNOUNCE: {
                    return QVariant{tracker.minAnnounce};
                }
            }
        }
    }
    return {};
}

QVariant TrackerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }
    if (orientation == Qt::Horizontal) {
        switch ((TrackerFields)section) {
            case TrackerFields::URL: {
                return QVariant{"Url"};
            }
            case TrackerFields::TIER: {
                return QVariant{"Tier"};
            }
            case TrackerFields::STATUS: {
                return QVariant{"Status"};
            }
            case TrackerFields::SEEDS: {
                return QVariant{"Seeds"};
            }
            case TrackerFields::LEECHES: {
                return QVariant{"Leeches"};
            }
            case TrackerFields::MESSAGE: {
                return QVariant{"Message"};
            }
            case TrackerFields::NEXT_ANNOUNCE: {
                return QVariant{"Next Announce"};
            }
            case TrackerFields::MIN_ANNOUNCE: {
                return QVariant{"Min. Announce"};
            }
            default: {
                throw std::runtime_error("How did u get here");
            }
        }
    } else {
        return QVariant{section + 1};
    }
}

QHash<int, QByteArray> TrackerTableModel::roleNames() const
{
    return {};
}

// TODO: change to && or vec<tracker> trackers so i can move here
void TrackerTableModel::setTrackers(const QList<Tracker> &trackers)
{
    // TODO: нормально это написать убррать эту хуйнбю
    beginResetModel();
    m_trackers = trackers;
    endResetModel();
}

void TrackerTableModel::clearTrackers()
{
    beginResetModel();
    m_trackers.clear();

}
