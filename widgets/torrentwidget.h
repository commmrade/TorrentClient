#ifndef TORRENTWIDGET_H
#define TORRENTWIDGET_H

#include <QWidget>
#include "sessionmanager.h"
#include "torrentstablemodel.h"
#include "torrentitemdelegate.h"

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
    void setupTableView();

    Ui::TorrentWidget *ui;
    SessionManager m_sessionManager;

    TorrentsTableModel m_tableModel;
    TorrentItemDelegate m_tableDelegate;
};

#endif // TORRENTWIDGET_H
