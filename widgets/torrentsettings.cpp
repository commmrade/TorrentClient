#include "torrentsettings.h"
#include "ui_torrentsettings.h"
#include "sessionmanager.h"
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include "settingsvalues.h"
#include "dirs.h"

TorrentSettings::TorrentSettings(QWidget *parent) : BaseSettings(parent), ui(new Ui::TorrentSettings)
{
    ui->setupUi(this);

    QSettings settings;
    {
        int downloadSpeedLimit = settings
                                     .value(SettingsNames::SESSION_DOWNLOAD_SPEED_LIMIT,
                                            SettingsValues::SESSION_DOWNLOAD_SPEED_LIMIT)
                                     .toInt() / 1024;
        QSignalBlocker blocker{ui->downloadLimitSpin};
        ui->downloadLimitSpin->setValue(downloadSpeedLimit);
    }

    {
        int uploadSpeedLimit = settings
                                   .value(SettingsNames::SESSION_UPLOAD_SPEED_LIMIT,
                                          SettingsValues::SESSION_UPLOAD_SPEED_LIMIT)
                                   .toInt() / 1024;
        QSignalBlocker blocker{ui->uploadLimitSpin};
        ui->uploadLimitSpin->setValue(uploadSpeedLimit);
    }
    {
        QString defaultSavePath =
            settings
                .value(SettingsNames::SESSION_DEFAULT_SAVE_LOCATION,
                       QVariant{QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)})
                .toString();
        QSignalBlocker blocker{ui->savePathLineEdit};
        ui->savePathLineEdit->setText(defaultSavePath);
    }
    {
        bool dhtEnabled = settings.value(SettingsNames::PRIVACY_DHT_ENABLED, SettingsValues::PRIVACY_DHT_ENABLED_DEFAULT).toBool();
        QSignalBlocker blocker{ui->dhtCheck};
        ui->dhtCheck->setChecked(dhtEnabled);
    }

    {
        bool peerExEnabled = settings.value(SettingsNames::PRIVACY_PEEREX_ENABLED, SettingsValues::PRIVACY_PEEREX_ENABLED_DEFAULT).toBool();
        QSignalBlocker blocker{ui->peerExCheck};
        ui->peerExCheck->setChecked(peerExEnabled);
    }

    {
        bool localDiscEnabled = settings.value(SettingsNames::PRIVACY_LOCAL_PEER_DESC, SettingsValues::PRIVACY_LOCAL_PEER_DESC_DEFAULT).toBool();
        QSignalBlocker blocker{ui->localPeerDiscCheck};
        ui->localPeerDiscCheck->setChecked(localDiscEnabled);
    }
}

TorrentSettings::~TorrentSettings() { delete ui; }

void TorrentSettings::apply()
{
    auto &sessionManager = SessionManager::instance();
    QSettings settings;
    if (m_downloadLimitChanged)
    {
        auto downloadLimitValue = ui->downloadLimitSpin->value();
        sessionManager.setDownloadLimit(downloadLimitValue * 1024); // Convert to bytes

        settings.setValue(SettingsNames::SESSION_DOWNLOAD_SPEED_LIMIT, downloadLimitValue * 1024);
        m_downloadLimitChanged = false;
    }
    if (m_uploadLimitChanged)
    {
        auto uploadLimitValue = ui->uploadLimitSpin->value();
        sessionManager.setUploadLimit(uploadLimitValue * 1024); // Convert to bytes

        settings.setValue(SettingsNames::SESSION_UPLOAD_SPEED_LIMIT, uploadLimitValue * 1024);
        m_uploadLimitChanged = false;
    }
    if (m_savePathChanged) {
        QString defaultSavePath = ui->savePathLineEdit->text();
        settings.setValue(SettingsNames::SESSION_DEFAULT_SAVE_LOCATION, defaultSavePath);
        m_savePathChanged = false;
    }
    if (m_resetChanged) {
        resetApp();
        m_resetChanged = false;
    }
    if (m_dhtChanged) {
        bool val = ui->dhtCheck->isChecked();
        settings.setValue(SettingsNames::PRIVACY_DHT_ENABLED, {val});
        sessionManager.setDht(val);

        m_dhtChanged = false;
    }
    if (m_peerExChanged) {
        bool val = ui->peerExCheck->isChecked();
        settings.setValue(SettingsNames::PRIVACY_PEEREX_ENABLED, {val});

        m_peerExChanged = false;
    }
    if (m_localPeerDiscChanged) {
        bool val = ui->localPeerDiscCheck->isChecked();
        settings.setValue(SettingsNames::PRIVACY_LOCAL_PEER_DESC, {val});
        sessionManager.setLsd(val);

        m_localPeerDiscChanged = false;
    }
}

void TorrentSettings::on_downloadLimitSpin_valueChanged([[maybe_unused]] int arg1)
{
    m_downloadLimitChanged = true;
    emit optionChanged();
}


void TorrentSettings::on_uploadLimitSpin_valueChanged([[maybe_unused]] int arg1)
{
    m_uploadLimitChanged = true;
    emit optionChanged();
}


void TorrentSettings::on_resetSessionButton_clicked()
{
    int r = QMessageBox::warning(this, tr("Warning"), tr("Are you sure that you want to reset all the settings completelty?"), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
    if (r == QMessageBox::Ok ) {
        m_resetChanged = true;
        resetApp();
    }
}


void TorrentSettings::on_savePathButton_clicked()
{

    QString defaultSavePath =
        QFileDialog::getExistingDirectory(this, tr("Choose a new default save directory"));
    if (!defaultSavePath.isEmpty())
    {
        ui->savePathLineEdit->setText(defaultSavePath);
        m_savePathChanged = true;
        emit optionChanged();
    }
}


void TorrentSettings::on_dhtCheck_clicked(bool checked)
{
    m_dhtChanged = true;
    emit optionChanged();
}


void TorrentSettings::on_peerExCheck_clicked(bool checked)
{
    m_peerExChanged = true;
    emit optionChanged();
}


void TorrentSettings::on_localPeerDiscCheck_clicked(bool checked)
{
    m_localPeerDiscChanged = true;
    emit optionChanged();
}

void TorrentSettings::resetApp()
{
    auto& sessionManager = SessionManager::instance();
    QSettings settings;
    sessionManager.resetSessionParams();
    settings.clear();
    settings.sync();

    auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir logs{basePath + QDir::separator() + Dirs::LOGS};
    QDir metadata{basePath + QDir::separator() + Dirs::METADATA};
    QDir state{basePath + QDir::separator() + Dirs::STATE};
    // QDir themes{basePath + QDir::separator() + Dirs::THEMES};
    QDir torrents{basePath + QDir::separator() + Dirs::TORRENTS};

    int result = logs.removeRecursively() +
                metadata.removeRecursively() +
                state.removeRecursively() +
                // themes.removeRecursively() +
                 torrents.removeRecursively();
    if (result < 5) {
        qFatal() << "Failed to remove application's files, it may be broken now in some ways";
    }
}

