#ifndef TORRENTWIDGET_H
#define TORRENTWIDGET_H

#include <QWidget>
#include "sessionmanager.h"
#include "torrentstablemodel.h"
#include "torrentitemdelegate.h"
#include <QPointer>
#include "categorysortfilter.h"

namespace Ui
{
class TorrentWidget;
}

class SpeedGraphWidget;
class QSystemTrayIcon;
class TorrentWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit TorrentWidget(QWidget *parent = nullptr);
    ~TorrentWidget();

  private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void customContextMenu(const QPoint &pos);

    void on_togglePropertiesBtn_clicked();

    void on_toggleGraphsButton_clicked();

    void closeAllTabs();

    void on_categoriesList_currentTextChanged(const QString &currentText);
    void torrentClicked(const QModelIndex& index);
  private:
    void setupTableView();
    void setupTray();
    void setupSession();

    void addTorrentByMagnet(const QString& magnetUri);
    void addTorrentByFile(const QString& filepath);

    Ui::TorrentWidget *ui;

    QPointer<SpeedGraphWidget> m_speedGraph;

    SessionManager &m_sessionManager;

    TorrentsTableModel  m_tableModel;
    TorrentItemDelegate m_tableDelegate;
    CategorySortFilter  m_categoryFilter;
    QSystemTrayIcon* m_trayIcon;
};

#endif // TORRENTWIDGET_H
