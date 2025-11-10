#include "mainwindow.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include "core/utils/settingsvalues.h"
#include <QDebug>
#include "core/utils/dirs.h"
#include <QMutex>
#include <QMutexLocker>
#include <QLockFile>
#include <iostream>

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

    settings.setValue(SettingsNames::GUI_THEME, SettingsValues::GUI_THEME_DARK);
    settings.remove(SettingsNames::GUI_CUSTOM_THEME);
}

struct f_deleter
{
    void operator()(std::FILE *f)
    {
        std::fflush(f);
        std::fclose(f);
    }
};

QtMessageHandler originalHandler;
bool             isLogLocked = false;
void             myMessageHandler(QtMsgType t, const QMessageLogContext &ctx, const QString &text)
{

    QByteArray localMsg         = text.toLocal8Bit();
    QString    curDatetime      = QDateTime::currentDateTime().toString();
    QByteArray curDatePrintable = curDatetime.toLocal8Bit();

    QSettings settings;
    QString   logsPath =
        settings
            .value(SettingsNames::LOGS_PATH,
                   QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                       QDir::separator() + Dirs::LOGS + QDir::separator())
            .toString() +
        QDir::separator() + "torrentclient.log";

    static QFile f{logsPath};
    if (!f.isOpen())
    {
        if (!f.open(QFile::WriteOnly))
        {
            qInstallMessageHandler(originalHandler);
            qCritical() << "Could not open a log file";
            return;
        }
    }

    long         seekPos = f.pos();
    unsigned int logMaxSize =
        settings.value(SettingsNames::LOGS_MAX_SIZE, SettingsValues::LOGS_MAX_SIZE_DEFAULT)
            .toUInt();
    if (seekPos > logMaxSize)
    {
        f.flush();
        f.close();

        if (!f.open(QFile::WriteOnly | QFile::Truncate))
        {
            qInstallMessageHandler(originalHandler);
            qCritical() << "Could not reopen a log file";
            return;
        }
    }

    QString msg;
    switch (t)
    {
    case QtDebugMsg:
    {
        msg = QString{"Debug [%1]: %2\n"}.arg(curDatePrintable.constData(), localMsg.constData());
        break;
    }
    case QtInfoMsg:
    {
        msg = QString{"Info [%1]: %2\n"}.arg(curDatePrintable.constData(), localMsg.constData());
        break;
    }
    case QtWarningMsg:
    {
        msg = QString{"Warning [%1]: %2\n"}.arg(curDatePrintable.constData(), localMsg.constData());
        break;
    }
    case QtCriticalMsg:
    {
        msg =
            QString{"Critical [%1]: %2\n"}.arg(curDatePrintable.constData(), localMsg.constData());
        break;
    }
    case QtFatalMsg:
    {
        msg = QString{"Fatal [%1]: %2\n"}.arg(curDatePrintable.constData(), localMsg.constData());
        break;
    }
    }
    f.write(msg.toUtf8());
    std::cerr << msg.toStdString();
}

void initDirsAndFiles()
{
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

void loadTheme(QApplication &a)
{
    QSettings settings;
    auto      basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
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
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setOrganizationDomain(ORG_DOM);
    QCoreApplication::setApplicationName(APP_NAME);

    QLockFile lockfile(QDir::temp().absoluteFilePath("torrent-client.lock"));
    if (!lockfile.tryLock())
    {
        throw std::runtime_error("An instance of this application is already running");
    }

    QApplication a(argc, argv);

    initDirsAndFiles();

    // if lockfile is locked we're the only one writing to torrent client
    QSettings settings;
    bool      logsEnabled =
        settings.value(SettingsNames::LOGS_ENABLED, SettingsValues::LOGS_ENABLED_DEFAULT).toBool();
    if (logsEnabled)
    {
        originalHandler = qInstallMessageHandler(myMessageHandler);
    }

    loadTheme(a);

    MainWindow w;
    w.show();
    return a.exec();
}
