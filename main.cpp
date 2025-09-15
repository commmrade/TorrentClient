#include "mainwindow.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include "settingsvalues.h"
#include <QDebug>
#include "dirs.h"

void fallToDefaultTheme(QApplication& a, QSettings& settings) {
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto darkThemePath = basePath + QDir::separator() + Dirs::THEMES + QDir::separator() + "dark.qss";
    QFile file(darkThemePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        a.setStyleSheet(file.readAll());
        file.close();
    }

    settings.setValue(SettingsValues::GUI_THEME, "Dark"); // reset to default theme (factor out in a function)
    settings.remove(SettingsValues::GUI_CUSTOM_THEME);
}


int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("klewy");
    QCoreApplication::setOrganizationDomain("klewy.com");
    QCoreApplication::setApplicationName("TorrentClient");

    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (!QDir().mkpath(basePath)) {
        qDebug() << "Could not create base path for torrent client";
    }
    QDir().mkdir(basePath + QDir::separator() + Dirs::TORRENTS); // Directory for downloads by default
    QDir().mkdir(basePath + QDir::separator() + Dirs::TORRENTS); // Directory for storing state of torrent
    QDir().mkdir(basePath + QDir::separator() + Dirs::METADATA); // Options and this kinda stuff maybe?
    QDir().mkdir(basePath + QDir::separator() + Dirs::THEMES);


    QApplication a(argc, argv);
    QSettings settings;
    // Set theme
    QString theme = settings.value(SettingsValues::GUI_THEME, "Dark").toString();
    if (theme == "Dark") {
        auto darkThemePath = basePath + QDir::separator() + Dirs::THEMES + QDir::separator() + "dark.qss";
        QFile file(darkThemePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            a.setStyleSheet(file.readAll());
            file.close();
        }
    } else if (theme == "Light") {
        auto lightThemePath = basePath + QDir::separator() + Dirs::THEMES + QDir::separator() + "light.qss";
        QFile file(lightThemePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            a.setStyleSheet(file.readAll());
            file.close();
        }
    } else if (theme == "Custom") {
        QString customThemePath = settings.value(SettingsValues::GUI_CUSTOM_THEME).toString();
        if (!customThemePath.isEmpty()) {
            QFile file(customThemePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                a.setStyleSheet(file.readAll());
                file.close();;
            } else { // if cant open theme file
                qDebug() << "Could not open theme file";
                fallToDefaultTheme(a, settings);
            }
        } else {
            qDebug() << "loaded theme but its not set";
            fallToDefaultTheme(a, settings);
        }
    }
    // qDebug() << "Application is in" << QCoreApplication::applicationFilePath();

    MainWindow w;
    w.show();
    return a.exec();
}
