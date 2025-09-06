#ifndef TORRENTWIDGET_H
#define TORRENTWIDGET_H

#include <QWidget>
#include "sessionmanager.h"
#include "torrentstablemodel.h"
#include "torrentitemdelegate.h"
#include <QPointer>

namespace Ui {
class TorrentWidget;
}

class SpeedGraphWidget;

class TorrentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TorrentWidget(QWidget *parent = nullptr);
    ~TorrentWidget();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void customContextMenu(const QPoint& pos);

    void on_togglePropertiesBtn_clicked();

    void on_pushButton_3_clicked();

private:
    void setupTableView();

    Ui::TorrentWidget *ui;

    QPointer<SpeedGraphWidget> m_speedGraph;

    SessionManager& m_sessionManager;

    TorrentsTableModel m_tableModel;
    TorrentItemDelegate m_tableDelegate;

};

#endif // TORRENTWIDGET_H
