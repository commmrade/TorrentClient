#ifndef GENERALINFOWIDGET_H
#define GENERALINFOWIDGET_H

#include <QWidget>
#include "torrentinfo.h"
#include <libtorrent/libtorrent.hpp>

namespace Ui {
class GeneralInfoWidget;
}



class GeneralInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralInfoWidget(QWidget *parent = nullptr);
    ~GeneralInfoWidget();

    void setGeneralInfo(const TorrentInfo& tInfo, const InternetInfo& iInfo);
    void clearGeneralInfo();

    void setPieces(const lt::typed_bitfield<lt::piece_index_t>& pieces);
private:
    Ui::GeneralInfoWidget *ui;
};

#endif // GENERALINFOWIDGET_H
