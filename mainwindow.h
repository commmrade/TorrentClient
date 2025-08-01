#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include "torrentwidget.h"
#include "settingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionSettings_triggered();

private:
    Ui::MainWindow *ui;

    QPointer<TorrentWidget> m_torrentWidget;
    QPointer<SettingsDialog> m_settingsDialog;
};
#endif // MAINWINDOW_H
