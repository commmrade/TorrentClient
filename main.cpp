#include "mainwindow.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include "settingsvalues.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("klewy");
    QCoreApplication::setOrganizationDomain("klewy.com");
    QCoreApplication::setApplicationName("TorrentClient");

    QSettings settings;
    settings.setValue("a", "b");

    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkdir(basePath);
    QDir().mkdir(basePath + QDir::separator() + "torrents"); // Directory for downloads by default
    QDir().mkdir(basePath + QDir::separator() + "state"); // Directory for storing state of torrent
    QDir().mkdir(basePath + QDir::separator() + "metadata"); // Options and this kinda stuff maybe?



    QApplication a(argc, argv);

    // Set theme
    QString theme = settings.value(SettingsValues::GUI_THEME, "Dark").toString();
    if (theme == "Dark") {
        QFile file("dark.qss");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            a.setStyleSheet(file.readAll());
            file.close();
        }
    } else {
        QFile file("light.qss");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            a.setStyleSheet(file.readAll());
            file.close();
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
