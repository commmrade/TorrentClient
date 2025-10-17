#include "mainwindow.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include "settingsvalues.h"
#include <QDebug>
#include "dirs.h"
#include <iostream>
#include <QMutex>
#include <QMutexLocker>

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

    settings.setValue(
        SettingsNames::GUI_THEME,
        SettingsValues::GUI_THEME_DARK);
    settings.remove(SettingsNames::GUI_CUSTOM_THEME);
}


struct f_deleter {
    void operator()(std::FILE* f) {
        std::fflush(f);
        std::fclose(f);
    }
};

void myMessageHandler(QtMsgType t, const QMessageLogContext& ctx, const QString& text) {
    QByteArray localMsg = text.toLocal8Bit();
    auto curDatetime = QDateTime::currentDateTime();
    const char* curDatePrintable = qPrintable(curDatetime.toString());

    fprintf(stderr, "Debug [%s]: %s\n", curDatePrintable, localMsg.constData());

    QSettings settings;
    QString logsPath = settings.value(SettingsNames::LOGS_PATH, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + Dirs::LOGS + QDir::separator()).toString() + QDir::separator() + "torrentclient.log";
    static std::unique_ptr<std::FILE, f_deleter> f{std::fopen(qPrintable(logsPath), "a")};
    if (!f.get()) return;
    switch (t) {
        case QtDebugMsg:
            fprintf(f.get(), "Debug [%s]: %s\n", curDatePrintable, localMsg.constData());
            break;
        case QtInfoMsg:
            fprintf(f.get(), "Info [%s]: %s\n", curDatePrintable, localMsg.constData());
            break;
        case QtWarningMsg:
            fprintf(f.get(), "Warning [%s]: %s\n", curDatePrintable, localMsg.constData());
            break;
        case QtCriticalMsg:
            fprintf(f.get(), "Critical [%s]: %s\n", curDatePrintable, localMsg.constData());
            break;
        case QtFatalMsg:
            fprintf(f.get(), "Fatal [%s]: %s\n", curDatePrintable, localMsg.constData());
            break;
    }
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
    QDir().mkdir(basePath + QDir::separator() + Dirs::LOGS);



    QApplication a(argc, argv);
    qInstallMessageHandler(myMessageHandler);
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
    qDebug() << "Starting the app";
    w.show();
    return a.exec();
}
