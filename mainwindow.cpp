#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_torrentWidget(new TorrentWidget)
{
    ui->setupUi(this);
    setCentralWidget(m_torrentWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
