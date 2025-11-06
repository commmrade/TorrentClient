#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <QWidget>
#include <libtorrent/peer_info.hpp>
#include "core/utils/torrentinfo.h"
#include "core/utils/file.h"

namespace Ui
{
class PropertiesWidget;
}
struct Tracker;
class PropertiesWidget : public QWidget
{
    Q_OBJECT

    void setupPropertiesTabs();

  public:
    explicit PropertiesWidget(QWidget *parent = nullptr);
    ~PropertiesWidget();

  public slots:
    void setPeers(const std::uint32_t id, const std::vector<lt::peer_info> &peer);
    void clearPeers();
    void setGeneralInfo(const TorrentInfo &tInfo, const InternetInfo &iInfo);
    void clearGeneralInfo();
    void setTrackers(const QList<Tracker> &trackers);
    void clearTrackers();

    void setPieces(const lt::typed_bitfield<lt::piece_index_t> &pieces,
                   const std::vector<int>                      &downloadingPiecesIndices);

    void setUrlSeeds(const std::set<std::string> &urlSeeds);
    void clearUrlSeeds();

    void setFiles(const QList<File> &files);
    void clearFiles();

  private:
    Ui::PropertiesWidget *ui;
};

#endif // PROPERTIESWIDGET_H
