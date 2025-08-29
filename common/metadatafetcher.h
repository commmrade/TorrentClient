#ifndef METADATAFETCHER_H
#define METADATAFETCHER_H

#include <QObject>
#include <QThread>
#include <libtorrent/add_torrent_params.hpp>

class MetadataFetcher : public QThread
{
    Q_OBJECT
public:
    explicit MetadataFetcher(const lt::add_torrent_params& params, QObject *parent = nullptr) : QThread(parent), m_params(params) {}
    void stopRunning();
protected:
    void run() override;
signals:
    void sizeReady(std::int64_t bytes);
private:
    lt::add_torrent_params m_params;
    bool m_isRunning{true};
};

#endif // METADATAFETCHER_H
