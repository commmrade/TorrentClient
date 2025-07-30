#include "torrentstablemodel.h"


TorrentsTableModel::TorrentsTableModel(QObject *parent) : QAbstractTableModel(parent)
{
    m_torrents.append(Torrent {
        328473,
        "ArchLinux 28.0.07.iso",
        "1.4 GB",
        15.4,
        "Downloading",
        100,
        100,
        "22.7 MB/s",
        "160.23 KB/s"
    });
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

