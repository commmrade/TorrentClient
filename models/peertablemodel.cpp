#include "peertablemodel.h"
#include <QElapsedTimer>
#include <QThread>
#include "utils.h"

PeerTableModel::PeerTableModel(QObject *parent)
    : QAbstractTableModel{parent}
{
    int status = MMDB_open("GeoIP.mmdb", MMDB_MODE_MMAP, &m_mmdb);
    if (status != MMDB_SUCCESS) {
        qWarning("GeoIP Database was not found, defaulting to Unknown");
        m_mmdb.filename = nullptr; // Make sure mmdb context is not valid
    }
}

PeerTableModel::~PeerTableModel()
{
    if (m_mmdb.filename) {
        MMDB_close(&m_mmdb);
    }
}

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
                    QString sizeStr = bytesToHigherPerSec(sizeInBytes);
                    return QVariant{sizeStr};
                }
                case PeerFields::DOWN_SPEED: {
                    auto sizeInBytes = peer.downSpeed;
                    QString sizeStr = bytesToHigherPerSec(sizeInBytes);
                    return QVariant{sizeStr};
                }
                case PeerFields::DOWNLOADED: {
                    auto sizeInBytes = peer.downloaded;
                    QString sizeStr = bytesToHigher(sizeInBytes);
                    return QVariant{sizeStr};
                }
                case PeerFields::UPLOADED: {
                    auto sizeInBytes = peer.uploaded;
                    QString sizeStr = bytesToHigher(sizeInBytes);
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
    beginResetModel();
    m_peers.clear();
    endResetModel();
}

QString PeerTableModel::countryFromIp(QByteArrayView ip)
{
    QString countryName{"Unknown"};
    if (m_mmdb.filename) {
        int gai_status;
        int status;
        MMDB_lookup_result_s search_result = MMDB_lookup_string(&m_mmdb, ip.data(), &gai_status, &status);
        if (status != MMDB_SUCCESS) {
            qWarning() << "Could not lookup address" << ip;
            return countryName;
        }

        MMDB_entry_data_s entry_data;
        status = MMDB_get_value(&search_result.entry, &entry_data, "country", "names", "en", NULL);
        if (status == MMDB_SUCCESS && entry_data.has_data) {
            countryName.assign(entry_data.utf8_string, entry_data.utf8_string + entry_data.data_size);
        }
    }
    return countryName;
}

void PeerTableModel::setPeers(const std::vector<libtorrent::peer_info>& peers)
{
    auto conToStr = [](lt::connection_type_t t) -> QString {
        switch (t) {
        case lt::peer_info::standard_bittorrent: return "BT";
        case lt::peer_info::http_seed: return "HTTP";
        case lt::peer_info::web_seed: return "URL";
        default: return "UNKNOWN";
        }
    };
    auto makeKeyPeer = [](const Peer& peer) {
        return peer.ip + ":" + QString::number(peer.port);
    };
    auto makeKeyPeerInfo = [](const lt::peer_info& peer) {
        return QString::fromStdString(peer.ip.address().to_string()) + ":" + QString::number(peer.ip.port());
    };

    QHash<QString, int> newPeersMap; // key - row
    QHash<QString, int> oldPeersMap; // key - row

    // Cache peers

    QElapsedTimer perfTimer;
    perfTimer.start();
    for (auto i = 0; i < peers.size(); ++i) {
        QString const key = makeKeyPeerInfo(peers[i]);
        newPeersMap.insert(key, i);
    }
    for (auto i = 0; i < m_peers.size(); ++i) {
        QString const key = makeKeyPeer(m_peers[i]);
        oldPeersMap.insert(key, i);
    }


    // Handle inserts 'n updates
    for (auto i = 0; i < peers.size(); ++i) {
        auto newKey = makeKeyPeerInfo(peers[i]);
        if (oldPeersMap.contains(newKey)) {
            int row = oldPeersMap[newKey];
            auto& oldPeer = m_peers[row];

            float progress = peers[i].progress;
            quint64 upSpeed = static_cast<quint64>(peers[i].up_speed);
            quint64 downSpeed = static_cast<quint64>(peers[i].down_speed);
            quint64 downloaded = static_cast<quint64>(peers[i].total_download);
            quint64 uploaded = static_cast<quint64>(peers[i].total_upload);

            if (oldPeer.progress != progress ||
                oldPeer.upSpeed != upSpeed ||
                oldPeer.downSpeed != downSpeed ||
                oldPeer.downloaded != downloaded ||
                oldPeer.uploaded != uploaded)
            {
                oldPeer.progress = progress;
                oldPeer.upSpeed = upSpeed;
                oldPeer.downSpeed = downSpeed;
                oldPeer.downloaded = downloaded;
                oldPeer.uploaded = uploaded;

                // emit dataChanged(index(row, 6), index(row, columnCount() - 1));
            }
        } else {
            auto row = m_peers.size();
            beginInsertRows(QModelIndex(), row, row);

            auto ipStr = peers[i].ip.address().to_string();
            QString countryName = countryFromIp(ipStr);

            m_peers.append(Peer {
                countryName,
                QString::fromStdString(peers[i].ip.address().to_string()),
                peers[i].ip.port(),
                conToStr(peers[i].connection_type),
                QString::fromStdString(peers[i].client),
                peers[i].progress,
                static_cast<std::uint64_t>(peers[i].up_speed),
                static_cast<std::uint64_t>(peers[i].down_speed),
                static_cast<std::uint64_t>(peers[i].total_download),
                static_cast<std::uint64_t>(peers[i].total_upload)
            });
            endInsertRows();
        }
    }


    // Handle deleted rows, do it in the end, so it wont fuck up ids
    for (auto i = m_peers.size() - 1; i >= 0; --i) {
        QString const key = makeKeyPeer(m_peers[i]);
        if (!newPeersMap.contains(key)) { // if peer exists in old, but doesnt in new, it is deleted
            beginRemoveRows(QModelIndex(), i, i);
            m_peers.removeAt(i);
            endRemoveRows();
        }
    }

    if (m_peers.size())
        emit dataChanged(index(0, 5), index(m_peers.size() - 1, columnCount() - 1));
}
