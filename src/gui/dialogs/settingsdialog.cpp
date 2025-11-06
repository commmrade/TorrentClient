#include "gui/dialogs/settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include "core/controllers/sessionmanager.h"
#include "core/utils/settingsvalues.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDesktopServices>
#include "core/utils/dirs.h"
#include "gui/dialogs/managepeersdialog.h"
#include <QSignalBlocker>
#include "gui/widgets/applicationsettings.h"
#include "gui/widgets/torrentsettings.h"
#include "gui/widgets/connectionsettings.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , appSettings(new ApplicationSettings)
    , torSettings(new TorrentSettings)
    , connSettings(new ConnectionSettings)
    , advSettings(new AdvancedSettings)
{
    setFixedSize(900, 600);

    ui->setupUi(this);
    ui->applyButton->setEnabled(false);

    ui->stackedWidget->addWidget(appSettings.get());
    ui->stackedWidget->addWidget(torSettings.get());
    ui->stackedWidget->addWidget(connSettings.get());
    ui->stackedWidget->addWidget(advSettings.get());

    connect(appSettings, &BaseSettings::restartRequired, this, &SettingsDialog::onRestartRequired);
    connect(torSettings, &BaseSettings::restartRequired, this, &SettingsDialog::onRestartRequired);
    connect(connSettings, &BaseSettings::restartRequired, this, &SettingsDialog::onRestartRequired);
    connect(advSettings, &BaseSettings::restartRequired, this, &SettingsDialog::onRestartRequired);

    connect(appSettings, &BaseSettings::optionChanged, this, &SettingsDialog::onOptionChanged);
    connect(torSettings, &BaseSettings::optionChanged, this, &SettingsDialog::onOptionChanged);
    connect(connSettings, &BaseSettings::optionChanged, this, &SettingsDialog::onOptionChanged);
    connect(advSettings, &BaseSettings::optionChanged, this, &SettingsDialog::onOptionChanged);
}

SettingsDialog::~SettingsDialog() { delete ui; }

void SettingsDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void SettingsDialog::onRestartRequired() { m_restartRequired = true; }

void SettingsDialog::onOptionChanged()
{
    ui->applyButton->setEnabled(true);
    m_optionChanged = true;
}

void SettingsDialog::on_applyButton_clicked()
{
    appSettings->apply();
    torSettings->apply();
    connSettings->apply();
    advSettings->apply();

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

void SettingsDialog::on_okButton_clicked()
{
    on_applyButton_clicked();
    close();
}

void SettingsDialog::on_closeButton_clicked() { close(); }
