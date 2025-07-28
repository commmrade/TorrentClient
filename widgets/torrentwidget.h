#ifndef TORRENTWIDGET_H
#define TORRENTWIDGET_H

#include <QWidget>
#include "sessionmanager.h"

namespace Ui {
class TorrentWidget;
}

class TorrentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TorrentWidget(QWidget *parent = nullptr);
    ~TorrentWidget();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::TorrentWidget *ui;
    SessionManager m_sessionManager;
};

#endif // TORRENTWIDGET_H
