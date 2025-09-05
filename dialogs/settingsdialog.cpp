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

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // Default size
    this->setFixedSize(900, 600);


    // Disable cutom theme stuff
    ui->chooseThemeBtn->setVisible(false);
    ui->customThemeEdit->setVisible(false);

    QSettings settings;

    // Application category
    QString language = settings.value(SettingsValues::GUI_LANGUAGE, QString{"English"}).toString();
    ui->languageBox->setCurrentText(language);


    QString theme = settings.value(SettingsValues::GUI_THEME, QString{"Dark"}).toString();
    ui->themeBox->setCurrentText(theme);
    if (theme == "Custom") { // if theme is custom set custom theme edit, if empty set to dark (default)
        QString customTheme = settings.value("gui/customTheme").toString();
        qDebug() << "Custom theme found";
        if (!customTheme.isEmpty()) { // Weird
            ui->customThemeEdit->setText(customTheme);
        } else {
            qDebug() << "Loaded custom theme but its empty";
            ui->themeBox->setCurrentText("Dark"); // default theme is dark
        }
    }


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

// void SettingsDialog::on_customizeButton_clicked()
// {
//     QSettings settings;
//     auto themeString = settings.value(SettingsValues::GUI_THEME, "Dark");

//     auto basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

//     if (themeString == "Dark") {
//         // open dark.qss or whatevuh
//         auto darkThemePath = basePath + QDir::separator() + "themes" + QDir::separator() + "dark.qss";
//         if (!QDesktopServices::openUrl("file:" + darkThemePath)) {
//             qCritical("Could not open .qss file");
//         }
//     } else {
//         // open light.qss or whatever
//         auto lightThemePath = basePath + QDir::separator() + "themes" + QDir::separator() + "light.qss";
//         if (!QDesktopServices::openUrl("file:" + lightThemePath)) {
//             qCritical("Could not open .qss file");
//         }
//     }
// }

void SettingsDialog::on_applyButton_clicked()
{
    applyApplicationSettings();
    applyTorrentSettings();

    // Prompt for restart after applied all the settings
    if (m_restartRequired) {
        // Prompt
        int ret = QMessageBox::information(this, "Restart required", "You may need to restart the app to apply new settings, do it now?", QMessageBox::Apply | QMessageBox::Cancel);
        if (ret == QMessageBox::Apply) {
            QString const binaryPath = QCoreApplication::applicationFilePath();
            QProcess::startDetached(binaryPath);
            QApplication::exit(); // Kill old app process and start a detached new one
        }

        m_restartRequired = false;
    }
}

void SettingsDialog::applyApplicationSettings()
{
    QSettings settings;
    // Language
    if (m_languageChanged) {
        settings.setValue(SettingsValues::GUI_LANGUAGE, ui->languageBox->currentText());
        m_languageChanged = false;
    }
    // Theme
    if (m_themeChanged) {
        auto theme = ui->themeBox->currentText();
        if (theme == "Custom") {
            qDebug() << "CUstom theme";
            settings.setValue(SettingsValues::GUI_THEME, theme); // Remove theme
            settings.setValue("gui/customTheme", ui->customThemeEdit->text());
        } else {
            settings.setValue(SettingsValues::GUI_THEME, theme);
        }

        m_themeChanged = false;
    }
}

void SettingsDialog::applyTorrentSettings()
{
    auto& sessionManager = SessionManager::instance();

    if (m_downloadLimitChanged) {
        auto downloadLimitValue = ui->downloadLimitSpin->value();
        sessionManager.setDownloadLimit(downloadLimitValue);
        m_downloadLimitChanged = false;
    }

    if (m_uploadLimitChanged) {
        auto uploadLimitValue = ui->uploadLimitSpin->value();
        sessionManager.setUploadLimit(uploadLimitValue);
        m_uploadLimitChanged = false;
    }
}


void SettingsDialog::on_savePathButton_clicked()
{
    QString defaultSavePath = QFileDialog::getExistingDirectory(this, "Choose a new default save directory");
    if (!defaultSavePath.isEmpty()) {
        QSettings settings;
        settings.setValue(SettingsValues::SESSION_DEFAULT_SAVE_LOCATION, defaultSavePath);
        ui->savePathLineEdit->setText(defaultSavePath);
    }
}

void SettingsDialog::on_languageBox_currentTextChanged(const QString &arg1)
{
    m_languageChanged = true;
    m_restartRequired = true; // Can't change language in runtime, (actually i can)
}

void SettingsDialog::on_themeBox_currentTextChanged(const QString &arg1)
{
    if (arg1 == "Custom") {
        ui->customThemeEdit->setVisible(true);
        ui->chooseThemeBtn->setVisible(true);
    } else {
        ui->customThemeEdit->setVisible(false);
        ui->chooseThemeBtn->setVisible(false);
    }

    m_themeChanged = true;
    m_restartRequired = true; // Can't change theme in runtime (i think)
}

void SettingsDialog::on_downloadLimitSpin_valueChanged(int arg1)
{
    m_downloadLimitChanged = true;
}

void SettingsDialog::on_uploadLimitSpin_valueChanged(int arg1)
{
    m_uploadLimitChanged = true;
}

void SettingsDialog::on_okButton_clicked()
{
    on_applyButton_clicked();
    close();
}


void SettingsDialog::on_closeButton_clicked()
{
    close();
}


void SettingsDialog::on_chooseThemeBtn_clicked()
{
    auto homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString themePath = QFileDialog::getOpenFileName(this, "Style file", homePath, "Style (*.qss)");

    if (!themePath.isEmpty()) {
        ui->customThemeEdit->setText(themePath);
        m_themeChanged = true;
    }
}

