#include "peertablemodel.h"

PeerTableModel::PeerTableModel(QObject *parent)
    : QAbstractTableModel{parent}
{}

inline double ceilTwoAfterComa(double number) {
    return std::ceil(number * 100.0) / 100.0;
}


QVariant PeerTableModel::data(const QModelIndex &index, int role) const
{
    if (role < Qt::UserRole) {
        if (role == Qt::DisplayRole) {
            auto& peer = m_peers[index.row()];
            switch ((PeerFields)index.column()) {
                case PeerFields::COUNTRY: {
                    return peer.country;
                }
                case PeerFields::IP: {
                    return peer.ip;
                }
                case PeerFields::PORT: {
                    return peer.port;
                }
                case PeerFields::CONNECTION: {
                    return peer.connectionType;
                }
                case PeerFields::CLIENT: {
                    return peer.client;
                }
                case PeerFields::PROGRESS: {
                    return QString(QString::number(peer.progress) + "%");
                }
                case PeerFields::UP_SPEED: {
                    auto sizeInBytes = peer.upSpeed;
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
                case PeerFields::DOWN_SPEED: {
                    auto sizeInBytes = peer.downSpeed;
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
                case PeerFields::DOWNLOADED: {
                    auto sizeInBytes = peer.downloaded;
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
                case PeerFields::UPLOADED: {
                    auto sizeInBytes = peer.uploaded;
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
                default: {
                    throw std::runtime_error{"Something is wrong"};
                }
            }
        }
    }
    return {};
}

QVariant PeerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        switch ((PeerFields)section) {
            case PeerFields::COUNTRY: {
                return QVariant{"Country"};
            }
            case PeerFields::IP: {
                return QVariant{"IP"};
            }
            case PeerFields::PORT: {
                return QVariant{"Port"};
            }
            case PeerFields::CONNECTION: {
                return QVariant{"Connection"};
            }
            case PeerFields::CLIENT: {
                return QVariant{"Client"};
            }
            case PeerFields::PROGRESS: {
                return QVariant{"Progress"};
            }
            case PeerFields::UP_SPEED: {
                return QVariant{"Up Speed"};
            }
            case PeerFields::DOWN_SPEED: {
                return QVariant{"Down Speed"};
            }
            case PeerFields::DOWNLOADED: {
                return QVariant{"Downloaded"};
            }
            case PeerFields::UPLOADED: {
                return QVariant{"Uploaded"};
            }
            default: {
                throw std::runtime_error{"Something is wrong"};
            }
        }
    } else {
        return QVariant{section + 1};
    }
}

QHash<int, QByteArray> PeerTableModel::roleNames() const
{
    return {};
}

void PeerTableModel::clearPeers()
{
    beginRemoveRows(QModelIndex(), 0, m_peers.size());
    m_peers.clear();
    endRemoveRows();
}

void PeerTableModel::setPeers(std::vector<libtorrent::peer_info> peers)
{
    auto conToStr = [](lt::connection_type_t t) -> QString {
        switch (t) {
        case lt::peer_info::standard_bittorrent: return "BT";
        case lt::peer_info::http_seed:           return "HTTP";
        case lt::peer_info::web_seed:            return "URL";
        default:                                 return "UNKNOWN";
        }
    };

    beginResetModel();
    m_peers.clear();
    m_peers.reserve(peers.size());

    for (const auto& peer : peers) {
        m_peers.append(Peer {
            "Ukraine",  // ← TODO: Replace with actual country if possible
            QString::fromStdString(peer.ip.address().to_string()),
            peer.ip.port(),
            conToStr(peer.connection_type),
            QString::fromStdString(peer.client),
            peer.progress,
            static_cast<std::uint64_t>(peer.up_speed),
            static_cast<std::uint64_t>(peer.down_speed),
            static_cast<std::uint64_t>(peer.total_download),
            static_cast<std::uint64_t>(peer.total_upload)
        });
    }

    endResetModel();
}
