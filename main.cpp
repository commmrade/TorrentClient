#include "mainwindow.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include "settingsvalues.h"
#include <QDebug>
#include "dirs.h"
#include <QMutex>
#include <QMutexLocker>
#include <QLockFile>

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


QtMessageHandler originalHandler;
bool isLogLocked = false;
void myMessageHandler(QtMsgType t, const QMessageLogContext& ctx, const QString& text) {
    QSettings settings;
    bool logsEnabled = settings.value(SettingsNames::LOGS_ENABLED, SettingsValues::LOGS_ENABLED_DEFAULT).toBool();
    if (!logsEnabled) {
        originalHandler(t, ctx, text);
        return;
    }

    QByteArray localMsg = text.toLocal8Bit();
    QString curDatetime = QDateTime::currentDateTime().toString();
    QByteArray curDatePrintable = curDatetime.toLocal8Bit();

    fprintf(stderr, "Debug [%s]: %s\n", curDatePrintable.constData(), localMsg.constData());

    QString logsPath = settings.value(SettingsNames::LOGS_PATH, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + Dirs::LOGS + QDir::separator()).toString() + QDir::separator() + "torrentclient.log";
    static QLockFile lock{logsPath + ".lock"}; // TODO: Is there a better way?, its kinda expensive to lock every write
    if (!isLogLocked) {
        if (!lock.tryLock()) {
            fprintf(stderr, "Can't lock log file yet\n");
            return;
        } else {
            isLogLocked = true;
        }
    }

    static std::unique_ptr<std::FILE, f_deleter> f{std::fopen(qPrintable(logsPath), "a")};
    if (!f) {
        return; // how to handl this?
    }

    long seekPos = ftell(f.get());
    unsigned int logMaxSize = settings.value(SettingsNames::LOGS_MAX_SIZE, SettingsValues::LOGS_MAX_SIZE_DEFAULT).toUInt();
    if (seekPos > logMaxSize) {
        std::FILE* oldFile = f.release();
        std::FILE* newFile = std::freopen(NULL, "w", oldFile); // oldFile closed here
        f.reset(newFile);
        if (!f) {
            return;
        }
    }

    switch (t) {
        case QtDebugMsg:
            fprintf(f.get(), "Debug [%s]: %s\n", curDatePrintable.constData(), localMsg.constData());
            break;
        case QtInfoMsg:
            fprintf(f.get(), "Info [%s]: %s\n", curDatePrintable.constData(), localMsg.constData());
            break;
        case QtWarningMsg:
            fprintf(f.get(), "Warning [%s]: %s\n", curDatePrintable.constData(), localMsg.constData());
            break;
        case QtCriticalMsg:
            fprintf(f.get(), "Critical [%s]: %s\n", curDatePrintable.constData(), localMsg.constData());
            break;
        case QtFatalMsg:
            fprintf(f.get(), "Fatal [%s]: %s\n", curDatePrintable.constData(), localMsg.constData());
            break;
    }
}


void initDirsAndFiles() {
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

}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setOrganizationDomain(ORG_DOM);
    QCoreApplication::setApplicationName(APP_NAME);

    QLockFile lockfile(QDir::temp().absoluteFilePath("torrent-client.lock"));
    if (!lockfile.tryLock()) {
        throw std::runtime_error("An instance of this application is already running");
    }

    initDirsAndFiles();
    QApplication a(argc, argv);
    originalHandler = qInstallMessageHandler(myMessageHandler);
    QSettings    settings;
    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    // Set theme
    int theme = settings.value(SettingsNames::GUI_THEME, SettingsValues::GUI_THEME_DARK).toInt();
    if (theme == SettingsValues::GUI_THEME_DARK)
    {
        // auto darkThemePath =
        //     basePath + QDir::separator() + Dirs::THEMES + QDir::separator() + "dark.qss";
        QFile file(":/dark.qss");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            a.setStyleSheet(file.readAll());
            file.close();
        }
    }
    else if (theme == SettingsValues::GUI_THEME_LIGHT)
    {
        // auto lightThemePath = Q
        //     basePath + QDir::separator() + Dirs::THEMES + QDir::separator() + "light.qss";
        QFile file(":/light.qss");
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
