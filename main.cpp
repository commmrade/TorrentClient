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
    QDir().mkdir(basePath + QDir::separator() + "themes");



    QApplication a(argc, argv);

    // Set theme
    QString theme = settings.value(SettingsValues::GUI_THEME, "Dark").toString();
    if (theme == "Dark") {
        auto darkThemePath = basePath + QDir::separator() + "themes" + QDir::separator() + "dark.qss";
        QFile file(darkThemePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            a.setStyleSheet(file.readAll());
            file.close();
        }
    } else {
        auto lightThemePath = basePath + QDir::separator() + "themes" + QDir::separator() + "light.qss";
        QFile file(lightThemePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            a.setStyleSheet(file.readAll());
            file.close();
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
