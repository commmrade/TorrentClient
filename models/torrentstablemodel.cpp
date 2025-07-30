#include "torrentstablemodel.h"
#include <QList>
#include <spdlog/spdlog.h>

TorrentsTableModel::TorrentsTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}

QVariant TorrentsTableModel::data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const
{
    if (role < Qt::UserRole) {
        if (role == Qt::DisplayRole) {
            auto& torrent = m_torrents[index.row()];
            switch (index.column()) {
                case ID: {
                    return torrent.id;
                    break;
                }
                case NAME: {
                    return torrent.name;
                    break;
                }
                case SIZE: {
                    return torrent.size;
                    break;
                }
                case PROGRESS: {
                    return torrent.progress;
                    break;
                }
                case STATUS: {
                    return torrent.status;
                    break;
                }
                case SEEDS: {
                    return torrent.seeds;
                    break;
                }
                case PEERS: {
                    return torrent.peers;
                    break;
                }
                case DOWN_SPEED: {
                    return torrent.downSpeed;
                    break;
                }
                case UP_SPEED: {
                    return torrent.upSpeed;
                    break;
                }
                default: {
                    throw std::runtime_error("Something is wrong");
                    break;
                }
            }
        }
    } else {
        // TODO: Handle custom roles
    }
    return {};
}

bool TorrentsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        auto& torrent = m_torrents[index.row()];
        switch (index.column()) {
            case ID: {
                // return torrent.id;
                torrent.id = value.toUInt();
                break;
            }
            case NAME: {
                // return torrent.name;
                torrent.name = value.toString();
                break;
            }
            case SIZE: {
                torrent.size = value.toString();
                // return torrent.size;
                break;
            }
            case PROGRESS: {
                torrent.progress = value.toDouble();
                // return torrent.progress;
                break;
            }
            case STATUS: {
                torrent.status = value.toString();
                // return torrent.status;
                break;
            }
            case SEEDS: {
                torrent.seeds = value.toInt();
                // return torrent.seeds;
                break;
            }
            case PEERS: {
                torrent.peers = value.toInt();
                // return torrent.peers;
                break;
            }
            case DOWN_SPEED: {
                torrent.downSpeed = value.toString();
                // return torrent.downSpeed;
                break;
            }
            case UP_SPEED: {
                torrent.upSpeed = value.toString();
                // return torrent.upSpeed;
                break;
            }
            default: {
                throw std::runtime_error("Something is wrong");
                break;
            }
        }
        return true;
    } else {
        // TODO: Handle Custom roles, i guess?
    }
    return false;
}

QVariant TorrentsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case ID: {
                return QVariant{"ID"};
            }
            case NAME: {
                return QVariant{"Name"};
            }
            case SIZE: {
                return QVariant{"Size"};
            }
            case PROGRESS: {
                return QVariant{"Progress"};
            }
            case STATUS: {
                return QVariant{"Status"};
            }
            case SEEDS: {
                return QVariant{"Seeds"};
            }
            case PEERS: {
                return QVariant{"Peers"};
            }
            case DOWN_SPEED: {
                return QVariant{"Down Speed"};
            }
            case UP_SPEED: {
                return QVariant{"Up Speed"};
            }
            default: {
                throw std::runtime_error("Something is wrong");
            }
        }
    } else {
        return QVariant{section + 1};
    }
}

QHash<int, QByteArray> TorrentsTableModel::roleNames() const
{
    // TODO: Implement custom roles
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

    torrentIterator->progress = 100.0;
    torrentIterator->status = torrentStateToString(status.state);
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
    m_torrents.remove(std::distance(m_torrents.begin(), torrentIterator));
    endRemoveRows();

    return true;
}

