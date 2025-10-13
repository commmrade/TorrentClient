#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "settingsdialog.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_torrentWidget(new TorrentWidget)
{
    ui->setupUi(this);

    setCentralWidget(m_torrentWidget);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog dialog(this);
    dialog.exec(); // idc if it is accepted or rejected
}

void MainWindow::changeEvent(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::WindowStateChange:
    {
        if (this->windowState() & Qt::WindowMinimized)
        {
            QTimer::singleShot(0, this, SLOT(hide()));
        }

        break;
    }
    default:
        break;
    }

    QMainWindow::changeEvent(event);
}

