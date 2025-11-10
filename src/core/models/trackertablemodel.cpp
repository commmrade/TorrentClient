#include "trackertablemodel.h"

TrackerTableModel::TrackerTableModel(QObject *parent) : QAbstractTableModel(parent) {}

QVariant TrackerTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (role < Qt::UserRole)
    {
        if (role == Qt::DisplayRole)
        {
            auto &tracker = m_trackers[index.row()];
            switch ((TrackerFields)index.column())
            {
            case TrackerFields::URL:
            {
                return QVariant{tracker.url};
            }
            case TrackerFields::TIER:
            {
                return QVariant{tracker.tier};
            }
            case TrackerFields::STATUS:
            {
                QString r = tracker.isWorking ? tr("Working") : tr("Stopped");
                return QVariant{r};
            }
            case TrackerFields::SEEDS:
            {
                return QVariant{tracker.seeds};
            }
            case TrackerFields::LEECHES:
            {
                return QVariant{tracker.leeches};
            }
            case TrackerFields::MESSAGE:
            {
                return QVariant{tracker.message};
            }
            case TrackerFields::NEXT_ANNOUNCE:
            {
                return QVariant{tracker.nextAnnounce};
            }
            case TrackerFields::MIN_ANNOUNCE:
            {
                return QVariant{tracker.minAnnounce};
            }
            }
        }
    }
    return {};
}

QVariant TrackerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return {};
    }
    if (orientation == Qt::Horizontal)
    {
        switch ((TrackerFields)section)
        {
        case TrackerFields::URL:
        {
            return QVariant{tr("Url")};
        }
        case TrackerFields::TIER:
        {
            return QVariant{tr("Tier")};
        }
        case TrackerFields::STATUS:
        {
            return QVariant{tr("Status")};
        }
        case TrackerFields::SEEDS:
        {
            return QVariant{tr("Seeds")};
        }
        case TrackerFields::LEECHES:
        {
            return QVariant{tr("Leeches")};
        }
        case TrackerFields::MESSAGE:
        {
            return QVariant{tr("Message")};
        }
        case TrackerFields::NEXT_ANNOUNCE:
        {
            return QVariant{tr("Next Announce")};
        }
        case TrackerFields::MIN_ANNOUNCE:
        {
            return QVariant{tr("Min. Announce")};
        }
        default:
        {
            throw std::runtime_error("How did u get here");
        }
        }
    }
    else
    {
        return QVariant{section + 1};
    }
}

QHash<int, QByteArray> TrackerTableModel::roleNames() const { return {}; }

void TrackerTableModel::setTrackers(const QList<Tracker> &trackers)
{
    auto makeKeyTracker = [](const Tracker &tracker) { return tracker.url; };

    QHash<QString, int> newTrackersMap; // key - index in trackers
    QHash<QString, int> oldTrackersMap; // key - index in m_trackers

    // Cache trackers
    for (int i = 0; i < trackers.size(); ++i)
    {
        QString const key = makeKeyTracker(trackers[i]);
        newTrackersMap.insert(key, i);
    }
    for (int i = 0; i < m_trackers.size(); ++i)
    {
        QString const key = makeKeyTracker(m_trackers[i]);
        oldTrackersMap.insert(key, i);
    }

    for (int i = 0; i < trackers.size(); ++i)
    {
        QString const newKey = makeKeyTracker(trackers[i]);
        if (oldTrackersMap.contains(newKey))
        {
            // Tracker exists, check for updates
            int   row        = oldTrackersMap[newKey];
            auto &oldTracker = m_trackers[row];

            // Check if any properties have changed
            if (oldTracker.tier != trackers[i].tier ||
                oldTracker.isWorking != trackers[i].isWorking ||
                oldTracker.seeds != trackers[i].seeds ||
                oldTracker.leeches != trackers[i].leeches ||
                oldTracker.message != trackers[i].message ||
                oldTracker.nextAnnounce != trackers[i].nextAnnounce ||
                oldTracker.minAnnounce != trackers[i].minAnnounce)
            {
                // Update properties
                oldTracker.tier         = trackers[i].tier;
                oldTracker.isWorking    = trackers[i].isWorking;
                oldTracker.seeds        = trackers[i].seeds;
                oldTracker.leeches      = trackers[i].leeches;
                oldTracker.message      = trackers[i].message;
                oldTracker.nextAnnounce = trackers[i].nextAnnounce;
                oldTracker.minAnnounce  = trackers[i].minAnnounce;
            }
        }
        else
        {
            int row = m_trackers.size();
            beginInsertRows(QModelIndex(), row, row);
            m_trackers.append(trackers[i]);
            endInsertRows();
        }
    }

    // Handle deletions
    for (int i = m_trackers.size() - 1; i >= 0; --i)
    {
        QString const key = makeKeyTracker(m_trackers[i]);
        if (!newTrackersMap.contains(key))
        {
            beginRemoveRows(QModelIndex(), i, i);
            m_trackers.removeAt(i);
            endRemoveRows();
        }
    }

    if (!m_trackers.isEmpty())
    {
        emit dataChanged(index(0, 1), index(m_trackers.size() - 1, columnCount() - 1));
    }
}

void TrackerTableModel::clearTrackers()
{
    beginResetModel();
    m_trackers.clear();
    endResetModel();
}
