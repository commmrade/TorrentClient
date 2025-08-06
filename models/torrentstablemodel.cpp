#include "torrentstablemodel.h"
#include <QList>
#include <spdlog/spdlog.h>


double ceilTwoAfterComa(double number) {
    return std::ceil(number * 100.0) / 100.0;
}


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
                }
                case NAME: {
                    return torrent.name;
                }
                case SIZE: {
                    auto sizeInBytes = torrent.size;
                    QString sizeStr;
                    if (sizeInBytes < 1024 * 1024) { // if less than a kilobyte
                        sizeStr = QString::number(ceilTwoAfterComa(sizeInBytes / 1024.0)) + " KB";
                    } else if (sizeInBytes < 1024 * 1024 * 1024) {
                        sizeStr = QString::number(ceilTwoAfterComa(sizeInBytes / 1024.0 / 1024.0)) + " MB";
                    } else {
                        sizeStr = QString::number(ceilTwoAfterComa(sizeInBytes / 1024.0 / 1024.0 / 1024.0)) + " GB";
                    }
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
                    QString sizeStr;
                    if (sizeInBytes < 1024 * 1024) { // if less than a kilobyte
                        sizeStr = QString::number(ceilTwoAfterComa(sizeInBytes / 1024.0)) + " KB/s";
                    } else if (sizeInBytes < 1024 * 1024 * 1024) {
                        sizeStr = QString::number(ceilTwoAfterComa(sizeInBytes / 1024.0 / 1024.0)) + " MB/s";
                    } else {
                        sizeStr = QString::number(ceilTwoAfterComa(sizeInBytes / 1024.0 / 1024.0 / 1024.0)) + " GB/s";
                    }
                    return QVariant{sizeStr};
                }
                case UP_SPEED: {
                    auto sizeInBytes = torrent.upSpeed;
                    QString sizeStr;
                    if (sizeInBytes < 1024 * 1024) { // if less than a kilobyte
                        sizeStr = QString::number(ceilTwoAfterComa(sizeInBytes / 1024.0)) + " KB/s";
                    } else if (sizeInBytes < 1024 * 1024 * 1024) {
                        sizeStr = QString::number(ceilTwoAfterComa(sizeInBytes / 1024.0 / 1024.0)) + " MB/s";
                    } else {
                        sizeStr = QString::number(ceilTwoAfterComa(sizeInBytes / 1024.0 / 1024.0 / 1024.0)) + " GB/s";
                    }
                    return QVariant{sizeStr};
                }
                case ETA: {
                    auto etaSecs = torrent.eta;

                    auto hrs = etaSecs / 3600;
                    auto mins = etaSecs % 3600 / 60;
                    auto secs = etaSecs % 60;

                    QString etaStr;
                    if (etaSecs == -1) {
                        etaStr = "infinity";
                    } else {
                        etaStr = QString("%1:%2:%3").arg(hrs).arg(mins).arg(secs);
                    }
                    return etaStr;
                }
                default: {
                    throw std::runtime_error("Something is wrong");
                    break;
                }
            }
        }
    }
    return {};
}

bool TorrentsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        auto& torrent = m_torrents[index.row()];
        switch (index.column()) {
            case ID: {
                torrent.id = value.toUInt();
                break;
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
            case ETA: {
                return QVariant{"ETA"};
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

