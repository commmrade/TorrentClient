#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include "sessionmanager.h"
#include "settingsvalues.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDesktopServices>
#include "dirs.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // Default size
    this->setFixedSize(900, 600); // TODO: maybe get rid of it

    // Disable cutom theme stuff
    ui->chooseThemeBtn->setVisible(false);
    ui->customThemeEdit->setVisible(false);

    QSettings settings;

    // Application category
    int language =
        settings.value(SettingsNames::GUI_LANGUAGE, SettingsValues::GUI_LANGUAGE_ENGLISH).toInt();
    ui->languageBox->blockSignals(true);
    ui->languageBox->setCurrentIndex(language);
    ui->languageBox->blockSignals(false);

    ui->themeBox->blockSignals(true);
    int theme = settings.value(SettingsNames::GUI_THEME, SettingsValues::GUI_THEME_DARK).toInt();
    ui->themeBox->setCurrentIndex(theme);
    if (theme == SettingsValues::GUI_THEME_CUSTOM)
    { // if theme is custom set custom theme edit, if empty set to dark (default)
        QString customTheme = settings.value(SettingsNames::GUI_CUSTOM_THEME).toString();
        if (!customTheme.isEmpty())
        {
            ui->customThemeEdit->setText(customTheme);
        }
        else
        {
            ui->themeBox->setCurrentIndex(SettingsValues::GUI_THEME_DARK);
        }
    }
    ui->themeBox->blockSignals(false);

    // TODO: Factor out in functions
    // Torrent category
    int downloadSpeedLimit = settings
                                 .value(SettingsNames::SESSION_DOWNLOAD_SPEED_LIMIT,
                                        SettingsValues::SESSION_DOWNLOAD_SPEED_LIMIT)
                                 .toInt() /
                             1024;
    ui->downloadLimitSpin->setValue(downloadSpeedLimit);

    int uploadSpeedLimit = settings
                               .value(SettingsNames::SESSION_UPLOAD_SPEED_LIMIT,
                                      SettingsValues::SESSION_UPLOAD_SPEED_LIMIT)
                               .toInt() /
                           1024;
    ui->uploadLimitSpin->setValue(uploadSpeedLimit);

    QString defaultSavePath =
        settings
            .value(SettingsNames::SESSION_DEFAULT_SAVE_LOCATION,
                   QVariant{QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)})
            .toString();
    ui->savePathLineEdit->setText(defaultSavePath);

    bool confirmDeletion = settings
                               .value(SettingsNames::TRANSFER_CONFIRM_DELETION,
                                      SettingsValues::TRANSFER_CONFIRM_DELETION_DEFAULT)
                               .toBool();
    ui->confirmDelBox->setChecked(confirmDeletion);

    int exitBeh =
        settings.value(SettingsNames::DESKTOP_EXIT_BEH, SettingsValues::DESKTOP_EXIT_BEH_CLOSE)
            .toInt();
    ui->exitBehBtn->blockSignals(true);
    ui->exitBehBtn->setCurrentIndex(exitBeh);
    ui->exitBehBtn->blockSignals(false);

    bool showTray =
        settings.value(SettingsNames::DESKTOP_SHOW_TRAY, SettingsValues::DESKTOP_SHOW_TRAY_DEFAULT)
            .toBool();
    ui->showTrayBox->setChecked(showTray);

    bool showNotifs =
        settings
            .value(SettingsNames::DESKTOP_SHOW_NOTIFS, SettingsValues::DESKTOP_SHOW_NOTIFS_DEFAULT)
            .toBool();
    ui->enaleNotifBox->setChecked(showNotifs);

    QString logsPath = settings.value(SettingsNames::LOGS_PATH, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + Dirs::LOGS + QDir::separator()).toString();
    ui->logsPathEdit->setText(logsPath);

    unsigned int logsMax = settings.value(SettingsNames::LOGS_MAX_SIZE, SettingsValues::LOGS_MAX_SIZE_DEFAULT).toUInt();
    ui->maxLogFileSpinBox->setValue(logsMax / 1024);

    bool logsEnabled = settings.value(SettingsNames::LOGS_ENABLED, SettingsValues::LOGS_ENABLED_DEFAULT).toBool();
    ui->logsBox->setChecked(logsEnabled);


    /// Connection stuff
    int port = settings.value(SettingsNames::LISTENING_PORT, SettingsValues::LISTENING_PORT_DEFAULT).toInt();
    ui->portBox->setValue(port);

    int protocolType = settings.value(SettingsNames::LISTENING_PROTOCOL, SettingsValues::LISTENING_PROTOCOL_TCP_AND_UTP).toInt();
    ui->peerConnProtocolBox->blockSignals(true);
    ui->peerConnProtocolBox->setCurrentIndex(protocolType);
    ui->peerConnProtocolBox->blockSignals(false);

    int maxNumOfCon = settings.value(SettingsNames::LIMITS_MAX_NUM_OF_CONNECTIONS, SettingsValues::LIMITS_MAX_NUM_OF_CONNECTIONS_DEFAULT).toInt();
    ui->mNumOfConBox->setValue(maxNumOfCon);
}

SettingsDialog::~SettingsDialog() { delete ui; }

void SettingsDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void SettingsDialog::on_applyButton_clicked()
{
    applyApplicationSettings();
    applyTorrentSettings();
    applyConnectionSettings();

    // Prompt for restart after applied all the settings
    if (m_restartRequired)
    {
        // Prompt
        int ret = QMessageBox::information(
            this, tr("Restart required"),
            tr("You may need to restart the app to apply new settings, do it now?"),
            QMessageBox::Apply | QMessageBox::Cancel);
        if (ret == QMessageBox::Apply)
        {
            QString const binaryPath = QCoreApplication::applicationFilePath();
            QApplication::quit(); // Kill old app process and start a detached new one
            QProcess::startDetached(binaryPath, qApp->arguments().mid(1));
        }
        m_restartRequired = false;
    }
}

void SettingsDialog::applyApplicationSettings()
{
    QSettings settings;
    // Language
    if (m_languageChanged)
    {
        settings.setValue(SettingsNames::GUI_LANGUAGE, ui->languageBox->currentIndex());
        m_languageChanged = false;
    }
    // Theme
    if (m_themeChanged)
    {
        auto theme = ui->themeBox->currentIndex();
        if (theme == SettingsValues::GUI_THEME_CUSTOM)
        {
            settings.setValue(SettingsNames::GUI_CUSTOM_THEME, ui->customThemeEdit->text());
        }
        settings.setValue(SettingsNames::GUI_THEME, theme);
        m_themeChanged = false;
    }
    if (m_confirmDeleteChanged) {
        bool checked = ui->confirmDelBox->isChecked();
        settings.setValue(SettingsNames::TRANSFER_CONFIRM_DELETION, checked);
        m_confirmDeleteChanged = false;
    }
    if (m_logsEnabledChanged) {
        settings.setValue(SettingsNames::LOGS_ENABLED, ui->logsBox->isChecked());
        m_logsEnabledChanged = false;
    }
    if (m_showTrayChanged) {
        bool checked = ui->showTrayBox->isChecked();
        settings.setValue(SettingsNames::DESKTOP_SHOW_TRAY, checked);
        if (!checked)
        {
            ui->enaleNotifBox->setChecked(false);
            on_enaleNotifBox_clicked(false);
            ui->exitBehBtn->setCurrentIndex(SettingsValues::DESKTOP_EXIT_BEH_CLOSE);
            on_exitBehBtn_currentIndexChanged(SettingsValues::DESKTOP_EXIT_BEH_CLOSE);
        }
        m_showTrayChanged = false;
    }
    if (m_enableNotifChanged) {
        bool checked = ui->enaleNotifBox->isChecked();
        if (checked && !ui->showTrayBox->isChecked())
        {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("You can't enable notifications when tray is disabled"));
            ui->enaleNotifBox->setChecked(false);
            return;
        }

        settings.setValue(SettingsNames::DESKTOP_SHOW_NOTIFS, checked);
        m_enableNotifChanged = false;
    }
    if (m_exitBehChanged) {
        int index = ui->exitBehBtn->currentIndex();
        if (index == SettingsValues::DESKTOP_EXIT_BEH_TO_TRAY && !ui->showTrayBox->isChecked())
        {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("You can't minimize to tray when tray is disabled"));
            ui->exitBehBtn->setCurrentIndex(SettingsValues::DESKTOP_EXIT_BEH_CLOSE);
            return;
        }
        settings.setValue(SettingsNames::DESKTOP_EXIT_BEH, index);
        m_exitBehChanged = false;
    }

    if (m_mLogSizeChanged) {
        int arg = ui->maxLogFileSpinBox->value();
        settings.setValue(SettingsNames::LOGS_MAX_SIZE, arg * 1024);

        m_mLogSizeChanged = false;
    }
    if (m_logsPathChanged) {
        QString logsPath = ui->logsPathEdit->text();
        settings.setValue(SettingsNames::LOGS_PATH, logsPath);
        m_logsPathChanged = false;
    }
}

void SettingsDialog::applyTorrentSettings()
{
    auto &sessionManager = SessionManager::instance();
    QSettings settings;
    if (m_downloadLimitChanged)
    {
        auto downloadLimitValue = ui->downloadLimitSpin->value();
        sessionManager.setDownloadLimit(downloadLimitValue * 1024); // Convert to bytes
        m_downloadLimitChanged = false;

        settings.setValue(SettingsNames::SESSION_DOWNLOAD_SPEED_LIMIT, QVariant{downloadLimitValue});
    }

    if (m_uploadLimitChanged)
    {
        auto uploadLimitValue = ui->uploadLimitSpin->value();
        sessionManager.setUploadLimit(uploadLimitValue * 1024); // Convert to bytes
        m_uploadLimitChanged = false;

        settings.setValue(SettingsNames::SESSION_UPLOAD_SPEED_LIMIT, QVariant{uploadLimitValue});
    }
    if (m_savePathChanged) {
        QString defaultSavePath = ui->savePathLineEdit->text();
        settings.setValue(SettingsNames::SESSION_DEFAULT_SAVE_LOCATION, defaultSavePath);
        m_savePathChanged = false;
    }
}

void SettingsDialog::applyConnectionSettings()
{
    auto& sessionManager = SessionManager::instance();
    QSettings settings;
    if (m_portChanged) {
        int port = ui->portBox->value();
        settings.setValue(SettingsNames::LISTENING_PORT, port);
        sessionManager.setListenPort(port);

        m_portChanged = false;
    }
    if (m_protocolChanged) {
        int protocolType = ui->peerConnProtocolBox->currentIndex();
        settings.setValue(SettingsNames::LISTENING_PROTOCOL, protocolType);
        sessionManager.setListenProtocol(protocolType);

        m_protocolChanged = false;
    }
    if (m_mNumOfConChanged) {
        int value = ui->mNumOfConBox->value();
        settings.setValue(SettingsNames::LIMITS_MAX_NUM_OF_CONNECTIONS, value);
        sessionManager.setMaxNumberOfConnections(value);

        m_mNumOfConChanged = false;
    }
}

void SettingsDialog::on_savePathButton_clicked()
{
    m_savePathChanged = true;
    QString defaultSavePath =
        QFileDialog::getExistingDirectory(this, tr("Choose a new default save directory"));
    if (!defaultSavePath.isEmpty())
    {
        ui->savePathLineEdit->setText(defaultSavePath);
    }
}

void SettingsDialog::on_downloadLimitSpin_valueChanged(int arg1) { m_downloadLimitChanged = true; }

void SettingsDialog::on_uploadLimitSpin_valueChanged(int arg1) { m_uploadLimitChanged = true; }

void SettingsDialog::on_okButton_clicked()
{
    on_applyButton_clicked();
    close();
}

void SettingsDialog::on_closeButton_clicked() { close(); }

void SettingsDialog::on_chooseThemeBtn_clicked()
{
    auto    homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString themePath =
        QFileDialog::getOpenFileName(this, tr("Style file"), homePath, tr("Style (*.qss)"));

    if (!themePath.isEmpty())
    {
        ui->customThemeEdit->setText(themePath);
        m_themeChanged = true;
    }
}

void SettingsDialog::on_confirmDelBox_clicked([[maybe_unused]] bool checked)
{
    m_confirmDeleteChanged = true;
}

void SettingsDialog::on_showTrayBox_clicked([[maybe_unused]] bool checked)
{
    m_showTrayChanged = true;
    m_restartRequired = true; // Cant enable tray in runtime (i mean i can but aint doin it
}

void SettingsDialog::on_enaleNotifBox_clicked([[maybe_unused]] bool checked)
{
    m_enableNotifChanged = true;
}

void SettingsDialog::on_exitBehBtn_currentIndexChanged([[maybe_unused]] int index)
{
    m_exitBehChanged = true;
}

void SettingsDialog::on_languageBox_currentIndexChanged([[maybe_unused]] int index)
{
    m_languageChanged = true;
    m_restartRequired = true; // Can't change language in runtime, (actually i can)
}

void SettingsDialog::on_themeBox_currentIndexChanged(int index)
{
    if (index == SettingsValues::GUI_THEME_CUSTOM)
    {
        ui->customThemeEdit->setVisible(true);
        ui->chooseThemeBtn->setVisible(true);
    }
    else
    {
        ui->customThemeEdit->setVisible(false);
        ui->chooseThemeBtn->setVisible(false);
    }

    m_themeChanged    = true;
    m_restartRequired = true; // Can't change theme in runtime (i think)
}

void SettingsDialog::on_logsPathBtn_clicked()
{
    auto    homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString logsPath =
        QFileDialog::getExistingDirectory(this, tr("Logs directory"), homePath);

    if (!logsPath.isEmpty()) {
        ui->logsPathEdit->setText(logsPath);
    }
    m_logsPathChanged = true;
    m_restartRequired = true;
}

void SettingsDialog::on_maxLogFileSpinBox_valueChanged([[maybe_unused]] int arg1)
{
    m_mLogSizeChanged = true;
}

void SettingsDialog::on_logsBox_clicked([[maybe_unused]] bool checked)
{
    m_logsEnabledChanged = true;
}


void SettingsDialog::on_portBox_valueChanged([[maybe_unused]] int arg1)
{
    m_portChanged = true;
}


void SettingsDialog::on_resetPortBtn_clicked()
{
    ui->portBox->setValue(SettingsValues::LISTENING_PORT_DEFAULT);
    on_portBox_valueChanged(SettingsValues::LISTENING_PORT_DEFAULT);
}


void SettingsDialog::on_peerConnProtocolBox_currentIndexChanged([[maybe_unused]] int index)
{
    m_protocolChanged = true;
}


void SettingsDialog::on_mNumOfConBox_valueChanged([[maybe_unused]] int arg1)
{
    m_mNumOfConChanged = true;
}


void SettingsDialog::on_mNumOfConPTBox_valueChanged([[maybe_unused]] int arg1)
{

}

