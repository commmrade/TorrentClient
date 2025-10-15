#include "mainwindow.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include "settingsvalues.h"
#include <QDebug>
#include "dirs.h"

static constexpr const char *ORG_NAME = "klewy";
static constexpr const char *ORG_DOM  = "klewy.com";
static constexpr const char *APP_NAME = "TorrentClient";

void fallToDefaultTheme(QApplication &a, QSettings &settings)
{
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto darkThemePath =
        basePath + QDir::separator() + Dirs::THEMES + QDir::separator() + "dark.qss";
    QFile file(darkThemePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        a.setStyleSheet(file.readAll());
        file.close();
    }

    settings.setValue(SettingsNames::GUI_THEME,
                      "Dark"); // reset to default theme (factor out in a function)
    settings.remove(SettingsNames::GUI_CUSTOM_THEME);
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setOrganizationDomain(ORG_DOM);
    QCoreApplication::setApplicationName(APP_NAME);

    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (!QDir().mkpath(basePath))
    {
        qDebug() << "Could not create base path for torrent client";
    }
    QDir().mkdir(basePath + QDir::separator() +
                 Dirs::TORRENTS); // Directory for downloads by default
    QDir().mkdir(basePath + QDir::separator() +
                 Dirs::TORRENTS); // Directory for storing state of torrent
    QDir().mkdir(basePath + QDir::separator() +
                 Dirs::METADATA); // Options and this kinda stuff maybe?
    QDir().mkdir(basePath + QDir::separator() + Dirs::THEMES);

    QApplication a(argc, argv);
    QSettings    settings;
    // Set theme
    int theme = settings.value(SettingsNames::GUI_THEME, SettingsValues::GUI_THEME_DARK).toInt();
    if (theme == SettingsValues::GUI_THEME_DARK)
    {
        auto darkThemePath =
            basePath + QDir::separator() + Dirs::THEMES + QDir::separator() + "dark.qss";
        QFile file(darkThemePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            a.setStyleSheet(file.readAll());
            file.close();
        }
    }
    else if (theme == SettingsValues::GUI_THEME_LIGHT)
    {
        auto lightThemePath =
            basePath + QDir::separator() + Dirs::THEMES + QDir::separator() + "light.qss";
        QFile file(lightThemePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            a.setStyleSheet(file.readAll());
            file.close();
        }
    }
    else if (theme == SettingsValues::GUI_THEME_CUSTOM)
    {
        QString customThemePath = settings.value(SettingsNames::GUI_CUSTOM_THEME).toString();
        if (!customThemePath.isEmpty())
        {
            QFile file(customThemePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                a.setStyleSheet(file.readAll());
                file.close();
            }
            else
            {
                qDebug() << "Could not open theme file";
                fallToDefaultTheme(a, settings);
            }
        }
        else
        {
            qDebug() << "loaded theme but its not set";
            fallToDefaultTheme(a, settings);
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
