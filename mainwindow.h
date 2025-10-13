#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include "speedgraphwidget.h"
#include "sessionmanager.h"
#include "torrentstablemodel.h"
#include "torrentitemdelegate.h"
#include "categorysortfilter.h"


QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class QSystemTrayIcon;
class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

  private slots:
    void on_actionSettings_triggered();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void customContextMenu(const QPoint &pos);

    void on_togglePropertiesBtn_clicked();

    void on_toggleGraphsButton_clicked();

    void closeAllTabs();

    void on_categoriesList_currentTextChanged(const QString &currentText);
    void torrentClicked(const QModelIndex& index);

    void changeEvent(QEvent* event) override;
  private:
    Ui::MainWindow *ui;

    // QPointer<TorrentWidget> m_torrentWidget;
    QPointer<SpeedGraphWidget> m_speedGraph;

    SessionManager &m_sessionManager;

    TorrentsTableModel  m_tableModel;
    TorrentItemDelegate m_tableDelegate;
    CategorySortFilter  m_categoryFilter;
    QSystemTrayIcon* m_trayIcon;

    void setupTableView();
    void setupTray();
    void setupSession();

    void addTorrentByMagnet(const QString& magnetUri);
    void addTorrentByFile(const QString& filepath);
};
#endif // MAINWINDOW_H
