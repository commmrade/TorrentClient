#include "mainwindow.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("klewy");
    QCoreApplication::setOrganizationDomain("klewy.com");
    QCoreApplication::setApplicationName("TorrentClient");

    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkdir(basePath);
    QDir().mkdir(basePath + QDir::separator() + "torrents"); // Directory for downloads by default
    QDir().mkdir(basePath + QDir::separator() + "state"); // Directory for storing state of torrent
    QDir().mkdir(basePath + QDir::separator() + "metadata"); // Options and this kinda stuff maybe?


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
