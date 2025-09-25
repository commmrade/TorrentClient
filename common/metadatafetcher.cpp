#include "metadatafetcher.h"
#include <libtorrent/session_params.hpp>
#include <libtorrent/alert.hpp>
#include <libtorrent/session.hpp>
#include <QDebug>
#include <libtorrent/alert_types.hpp>
#include <QTime>
#include "utils.h"

void MetadataFetcher::stopRunning()
{
    m_isRunning = false;
}

void MetadataFetcher::run()
{
    lt::session_params sessParams;
    sessParams.settings.set_int(
        lt::settings_pack::alert_mask,
        lt::alert_category::status |
            lt::alert_category::error |
            lt::alert_category::storage
        );
    m_session = std::make_unique<lt::session>(std::move(sessParams));
    m_session->add_torrent(std::move(m_params));
    sessionLoop();

    emit finished();
}

void MetadataFetcher::sessionLoop()
{
    while (m_isRunning) {
        std::vector<lt::alert*> alerts;
        m_session->pop_alerts(&alerts);
        for (auto* alert : alerts) {
            if (auto metadataRecAlert = lt::alert_cast<lt::metadata_received_alert>(alert)) {
                auto torrentFile = metadataRecAlert->handle.torrent_file();
                // auto ti = *torrentFile;

                // TorrentMetadata tmd;
                // auto totalSize = ti.total_size();
                // // ui->sizeInfo->setText(utils::bytesToHigher(totalSize));
                // tmd.size = totalSize;

                // auto creationTime = ti.creation_date();
                // auto tp = std::chrono::system_clock::from_time_t(creationTime);
                // auto castTime = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch());
                // auto datetime = QDateTime::fromSecsSinceEpoch(castTime.count());
                // tmd.creationTime = std::move(datetime);

                // auto hashv1 = ti.info_hashes().get(lt::protocol_version::V1);
                // auto hashv2 = ti.info_hashes().get(lt::protocol_version::V2);
                // auto hashbest = ti.info_hashes().get_best();

                // tmd.hashV1 = utils::toHex(hashv1.is_all_zeros() ? "" : hashv1.to_string());
                // tmd.hashV2 = utils::toHex(hashv2.is_all_zeros() ? "" : hashv2.to_string());
                // tmd.hashBest = utils::toHex(hashbest.is_all_zeros() ? "" : hashbest.to_string());

                // tmd.comment = QString::fromStdString(ti.comment());

                // emit metadataFetched(tmd);
                emit metadataFetched(torrentFile);
                m_isRunning = false;
                break;
            } else if (auto metadataFailedAlert = lt::alert_cast<lt::metadata_failed_alert>(alert)) {
                qWarning() << "Could not fetch metadata for magnet, because" << metadataFailedAlert->message();
                m_isRunning = false;
                emit error();
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
