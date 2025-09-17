#include "torrentstablemodel.h"
#include <QList>
#include <spdlog/spdlog.h>
#include "utils.h"

TorrentsTableModel::TorrentsTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}

QVariant TorrentsTableModel::data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const
{
    if (role < Qt::UserRole) {
        if (role == Qt::DisplayRole) {
            // if (std::abs(index.row()) >= m_torrents.size()) return {}; // TODO: what is this for
            // return {};
            auto& torrent = m_torrents[index.row()];

            auto id = index.column() + 2;
            switch (id) {
                case ID: {
                    return torrent.id;
                }
                case CATEGORY: {
                    return torrent.category;
                }
                case NAME: {
                    return torrent.name;
                }
                case SIZE: {
                    auto sizeInBytes = torrent.size;
                    QString sizeStr = utils::bytesToHigher(sizeInBytes);
                    return QVariant{sizeStr};
                }
                case PROGRESS: {
                    return torrent.progress;
                }
                case STATUS: {
                    return torrent.status;
                }
                case SEEDS: {
                    return torrent.seeds;
                }
                case PEERS: {
                    return torrent.peers;
                }
                case DOWN_SPEED: {
                    auto sizeInBytes = torrent.downSpeed;
                    QString sizeStr = utils::bytesToHigherPerSec(sizeInBytes);
                    return QVariant{sizeStr};
                }
                case UP_SPEED: {
                    auto sizeInBytes = torrent.upSpeed;
                    QString sizeStr = utils::bytesToHigherPerSec(sizeInBytes);
                    return QVariant{sizeStr};
                }
                case ETA: {
                    auto etaStr = utils::secsToFormattedTime(torrent.eta);
                    return etaStr;
                }
                default: {
                    throw std::runtime_error("Something is wrong");
                }
            }
        }
    } else { // For getting unseen data from sort filter proxy
        auto& torrent = m_torrents[index.row()];
        auto id = index.column();
        switch (id) {
            case ID: {
                return torrent.id;
            }
            case CATEGORY: {
                return torrent.category;
            }
        }
    }
    return {};
}

bool TorrentsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        auto& torrent = m_torrents[index.row()];
        switch (index.column() + 2) {
            case ID: {
                torrent.id = value.toUInt();
                break;
            }
            case CATEGORY: {
                torrent.category = value.toString();
            }
            case NAME: {
                torrent.name = value.toString();
                break;
            }
            case SIZE: {
                torrent.size = value.toULongLong();
                break;
            }
            case PROGRESS: {
                torrent.progress = value.toDouble();
                break;
            }
            case STATUS: {
                torrent.status = value.toString();
                break;
            }
            case SEEDS: {
                torrent.seeds = value.toInt();
                break;
            }
            case PEERS: {
                torrent.peers = value.toInt();
                break;
            }
            case DOWN_SPEED: {
                torrent.downSpeed = value.toULongLong();
                break;
            }
            case UP_SPEED: {
                torrent.upSpeed = value.toULongLong();
                break;
            }
            case ETA: {
                torrent.eta = value.toInt();
                break;
            }
            default: {
                throw std::runtime_error("Something is wrong");
                break;
            }
        }
        return true;
    }
    return false;
}

QVariant TorrentsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }
    if (orientation == Qt::Horizontal) {
        switch (section + 2) {
            // case ID: {
            //     return QVariant{"ID"};
            // }
            // case CATEGORY
            case NAME: {
                return QVariant{tr("Name")};
            }
            case SIZE: {
                return QVariant{tr("Size")};
            }
            case PROGRESS: {
                return QVariant{tr("Progress")};
            }
            case STATUS: {
                return QVariant{tr("Status")};
            }
            case SEEDS: {
                return QVariant{tr("Seeds")};
            }
            case PEERS: {
                return QVariant{tr("Peers")};
            }
            case DOWN_SPEED: {
                return QVariant{tr("Down Speed")};
            }
            case UP_SPEED: {
                return QVariant{tr("Up Speed")};
            }
            case ETA: {
                return QVariant{"ETA"};
            }
            default: {
                throw std::runtime_error("Something is wrong");
            }
        }
    } else {
        return QVariant{};
    }
    return {};
}

QHash<int, QByteArray> TorrentsTableModel::roleNames() const
{
    return {};
}

void TorrentsTableModel::addTorrent(const Torrent &torrent)
{
    auto newRowIdx = m_torrents.size();
    beginInsertRows(QModelIndex{}, newRowIdx, newRowIdx);
    m_torrents.append(torrent);
    endInsertRows();
}

bool TorrentsTableModel::updateTorrent(const Torrent &torrent)
{
    auto torrentIterator = std::find_if(m_torrents.begin(), m_torrents.end(), [id = torrent.id](const auto& torrent) {
        return torrent.id == id;
    });
    if (torrentIterator == m_torrents.end()) {
        spdlog::warn("Such torrent does not exist, id: {}", torrent.id);
        return false;
    }

    auto rowChanged = std::distance(m_torrents.begin(), torrentIterator);
    *torrentIterator = torrent;

    auto leftUp = index(rowChanged, 0);
    auto rightBottom = index(rowChanged, columnCount() - 1);
    emit dataChanged(leftUp, rightBottom);
    return true;
}

bool TorrentsTableModel::finishTorrent(const std::uint32_t id, const lt::torrent_status& status)
{
    auto torrentIterator = std::find_if(m_torrents.begin(), m_torrents.end(), [id](const auto& torrent) {
        return torrent.id == id;
    });
    if (torrentIterator == m_torrents.end()) {
        spdlog::warn("Such torrent does not exist, id: {}", id);
        return false;
    }

    // torrentIterator->progress = 100.0;
    // torrentIterator->status = torrentStateToString(status.state);
    return true;
}

bool TorrentsTableModel::removeTorrent(const std::uint32_t id)
{
    auto torrentIterator = std::find_if(m_torrents.begin(), m_torrents.end(), [id](const auto& torrent) {
        return torrent.id == id;
    });
    if (torrentIterator == m_torrents.end()) {
        spdlog::warn("Such torrent does not exist here, id: {}", id);
        return false;
    }

    auto rowIndex = std::distance(m_torrents.begin(), torrentIterator);
    beginRemoveRows(QModelIndex{}, rowIndex, rowIndex);
    m_torrents.remove(rowIndex);
    endRemoveRows();

    return true;
}

void TorrentsTableModel::setTorrentCategory(const uint32_t id, const QString &category)
{
    m_torrents[id].category = category;
}

