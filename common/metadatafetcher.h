#ifndef METADATAFETCHER_H
#define METADATAFETCHER_H

#include <QObject>
#include <QThread>
#include <libtorrent/add_torrent_params.hpp>
#include <memory>
#include <QDateTime>

struct TorrentMetadata {
    std::int64_t size;
    QDateTime creationTime;
    QString hashV1;
    QString hashV2;
    QString hashBest;
    QString comment;
};

class MetadataFetcher : public QObject
{
    Q_OBJECT
public:
    explicit MetadataFetcher(const lt::add_torrent_params& params, QObject *parent = nullptr) : QObject(parent), m_params(params) {}
    void stopRunning();
public:
    void run();
signals:
    void metadataFetched(std::shared_ptr<const lt::torrent_info> md);

    void error();
    void finished();
private:
    lt::add_torrent_params m_params;
    bool m_isRunning{true};
    std::unique_ptr<lt::session> m_session;

    void sessionLoop();
};

#endif // METADATAFETCHER_H
