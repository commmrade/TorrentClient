#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <QWidget>
#include <libtorrent/peer_info.hpp>
#include "torrentinfo.h"

namespace Ui {
class PropertiesWidget;
}

class PropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PropertiesWidget(QWidget *parent = nullptr);
    ~PropertiesWidget();

public slots:
    void setPeers(const std::uint32_t id, const std::vector<lt::peer_info>& peer);
    void clearPeers();
    void setGeneralInfo(const TorrentInfo& tInfo, const InternetInfo& iInfo);
    void clearGeneralInfo();
private:
    Ui::PropertiesWidget *ui;
};

#endif // PROPERTIESWIDGET_H
