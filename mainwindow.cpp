#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "settingsdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_torrentWidget(new TorrentWidget)
{
    ui->setupUi(this);

    setCentralWidget(m_torrentWidget);

    // test
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog dialog(this);
    dialog.exec(); // idc if it is accepted or rejected
}
