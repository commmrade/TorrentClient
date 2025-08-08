#ifndef GENERALINFOWIDGET_H
#define GENERALINFOWIDGET_H

#include <QWidget>
#include "torrentinfo.h"

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
private:
    Ui::GeneralInfoWidget *ui;
};

#endif // GENERALINFOWIDGET_H
