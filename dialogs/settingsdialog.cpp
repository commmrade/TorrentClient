#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include "sessionmanager.h"
#include "settingsvalues.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // TODO: Load settings from QSettings

    QSettings settings;

    // Application category
    QString language = settings.value(SettingsValues::GUI_LANGUAGE, QString{"English"}).toString();
    ui->languageBox->setCurrentText(language);

    QString theme = settings.value(SettingsValues::GUI_THEME, QString{"Dark"}).toString();
    ui->themeBox->setCurrentText(theme);

    // Torrent category
    int downloadSpeedLimit = settings.value(SettingsValues::SESSION_DOWNLOAD_SPEED_LIMIT, QVariant{0}).toInt() / 1024;
    ui->downloadLimitSpin->setValue(downloadSpeedLimit);

    int uploadSpeedLimit = settings.value(SettingsValues::SESSION_UPLOAD_SPEED_LIMIT, QVariant{0}).toInt() / 1024;
    ui->uploadLimitSpin->setValue(uploadSpeedLimit);

    QString defaultSavePath = settings.value(SettingsValues::SESSION_DEFAULT_SAVE_LOCATION,
                                            QVariant{QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)}).toString();
    ui->savePathLineEdit->setText(defaultSavePath);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void SettingsDialog::on_customizeButton_clicked()
{
    QSettings settings;
    // TODO: Open .qss file depending on theme
    auto themeString = settings.value(SettingsValues::GUI_THEME, "Dark");
    if (themeString == "Dark") {
        // open dark.qss or whatevuh
    } else {
        // open light.qss or whatever
    }
}

void SettingsDialog::on_applyButton_clicked()
{
    applyApplicationSettings();
    applyTorrentSettings();
}

void SettingsDialog::applyApplicationSettings()
{
    QSettings settings;
    // Language
    settings.setValue(SettingsValues::GUI_LANGUAGE, ui->languageBox->currentText());
    // Theme
    settings.setValue(SettingsValues::GUI_THEME, ui->themeBox->currentText());
}

void SettingsDialog::applyTorrentSettings()
{
    auto& sessionManager = SessionManager::instance();
    auto downloadLimitValue = ui->downloadLimitSpin->value();
    sessionManager.setDownloadLimit(downloadLimitValue);

    auto uploadLimitValue = ui->uploadLimitSpin->value();
    sessionManager.setUploadLimit(uploadLimitValue);
}


void SettingsDialog::on_savePathButton_clicked()
{
    // Change default save path
    // TODO: OPen file dialog and choose folder
}

