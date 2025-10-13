#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include "torrentwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

  private slots:
    void on_actionSettings_triggered();

    void changeEvent(QEvent* event) override;
  private:
    Ui::MainWindow *ui;

    QPointer<TorrentWidget> m_torrentWidget;
};
#endif // MAINWINDOW_H
