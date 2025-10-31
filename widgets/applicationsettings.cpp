#include "applicationsettings.h"
#include "ui_applicationsettings.h"
#include <QSettings>
#include "settingsvalues.h"
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include "dirs.h"

ApplicationSettings::ApplicationSettings(QWidget *parent)
    : BaseSettings(parent), ui(new Ui::ApplicationSettings)
{
    ui->setupUi(this);

    ui->chooseThemeBtn->setVisible(false);
    ui->customThemeEdit->setVisible(false);

    QSettings settings;
    {
        int language =
            settings.value(SettingsNames::GUI_LANGUAGE, SettingsValues::GUI_LANGUAGE_ENGLISH)
                .toInt();
        QSignalBlocker blocker{ui->languageBox};
        ui->languageBox->setCurrentIndex(language);
    }

    {
        QSignalBlocker blocker{ui->themeBox};
        int            theme =
            settings.value(SettingsNames::GUI_THEME, SettingsValues::GUI_THEME_DARK).toInt();
        ui->themeBox->setCurrentIndex(theme);
        if (theme == SettingsValues::GUI_THEME_CUSTOM)
        { // if theme is custom set custom theme edit, if empty set to dark (default)
            QString customTheme = settings.value(SettingsNames::GUI_CUSTOM_THEME).toString();
            if (!customTheme.isEmpty())
            {
                ui->customThemeEdit->blockSignals(true);
                ui->customThemeEdit->setText(customTheme);
                ui->customThemeEdit->blockSignals(false);
            }
            else
            {
                ui->themeBox->setCurrentIndex(SettingsValues::GUI_THEME_DARK);
            }
        }
    }

    {
        bool confirmDeletion = settings
                                   .value(SettingsNames::TRANSFER_CONFIRM_DELETION,
                                          SettingsValues::TRANSFER_CONFIRM_DELETION_DEFAULT)
                                   .toBool();
        QSignalBlocker blocker{ui->confirmDelBox};
        ui->confirmDelBox->setChecked(confirmDeletion);
    }

    {
        bool showTray =
            settings
                .value(SettingsNames::DESKTOP_SHOW_TRAY, SettingsValues::DESKTOP_SHOW_TRAY_DEFAULT)
                .toBool();
        QSignalBlocker blocker{ui->showTrayBox};
        ui->showTrayBox->setChecked(showTray);
    }

    {
        bool showNotifs = settings
                              .value(SettingsNames::DESKTOP_SHOW_NOTIFS,
                                     SettingsValues::DESKTOP_SHOW_NOTIFS_DEFAULT)
                              .toBool();
        QSignalBlocker blocker{ui->enaleNotifBox};
        ui->enaleNotifBox->setChecked(showNotifs);
    }

    {
        int exitBeh =
            settings.value(SettingsNames::DESKTOP_EXIT_BEH, SettingsValues::DESKTOP_EXIT_BEH_CLOSE)
                .toInt();
        QSignalBlocker blocker{ui->exitBehBtn};
        ui->exitBehBtn->setCurrentIndex(exitBeh);
    }

    {
        bool logsEnabled =
            settings.value(SettingsNames::LOGS_ENABLED, SettingsValues::LOGS_ENABLED_DEFAULT)
                .toBool();
        QSignalBlocker blocker{ui->logsBox};
        ui->logsBox->setChecked(logsEnabled);
    }
    {
        QString logsPath =
            settings
                .value(SettingsNames::LOGS_PATH,
                       QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                           QDir::separator() + Dirs::LOGS + QDir::separator())
                .toString();
        QSignalBlocker blocker{ui->logsPathEdit};
        ui->logsPathEdit->setText(logsPath);
    }
    {
        unsigned int logsMax =
            settings.value(SettingsNames::LOGS_MAX_SIZE, SettingsValues::LOGS_MAX_SIZE_DEFAULT)
                .toUInt();
        QSignalBlocker blocker{ui->maxLogFileSpinBox};
        ui->maxLogFileSpinBox->setValue(logsMax / 1024);
    }
}

ApplicationSettings::~ApplicationSettings() { delete ui; }

void ApplicationSettings::apply()
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
    if (m_confirmDeleteChanged)
    {
        bool checked = ui->confirmDelBox->isChecked();
        settings.setValue(SettingsNames::TRANSFER_CONFIRM_DELETION, checked);
        m_confirmDeleteChanged = false;
    }
    if (m_logsEnabledChanged)
    {
        settings.setValue(SettingsNames::LOGS_ENABLED, ui->logsBox->isChecked());
        m_logsEnabledChanged = false;
    }
    if (m_showTrayChanged)
    {
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
    if (m_enableNotifChanged)
    {
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
    if (m_exitBehChanged)
    {
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

    if (m_mLogSizeChanged)
    {
        int arg = ui->maxLogFileSpinBox->value();
        settings.setValue(SettingsNames::LOGS_MAX_SIZE, arg * 1024);

        m_mLogSizeChanged = false;
    }
    if (m_logsPathChanged)
    {
        QString logsPath = ui->logsPathEdit->text();
        settings.setValue(SettingsNames::LOGS_PATH, logsPath);
        m_logsPathChanged = false;
    }
}

void ApplicationSettings::on_languageBox_currentIndexChanged([[maybe_unused]] int index)
{
    m_languageChanged = true;
    emit restartRequired();
    emit optionChanged();
}

void ApplicationSettings::on_themeBox_currentIndexChanged([[maybe_unused]] int index)
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

    m_themeChanged = true;
    emit restartRequired();
    emit optionChanged();
}

void ApplicationSettings::on_chooseThemeBtn_clicked()
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

void ApplicationSettings::on_confirmDelBox_clicked([[maybe_unused]] bool checked)
{
    m_confirmDeleteChanged = true;
    emit optionChanged();
}

void ApplicationSettings::on_enaleNotifBox_clicked([[maybe_unused]] bool checked)
{
    m_enableNotifChanged = true;
    emit optionChanged();
}

void ApplicationSettings::on_exitBehBtn_currentIndexChanged([[maybe_unused]] int index)
{
    m_exitBehChanged = true;
    emit optionChanged();
}

void ApplicationSettings::on_logsBox_clicked([[maybe_unused]] bool checked)
{
    m_logsEnabledChanged = true;
    emit optionChanged();
}

void ApplicationSettings::on_logsPathBtn_clicked()
{
    auto    homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString logsPath = QFileDialog::getExistingDirectory(this, tr("Logs directory"), homePath);

    if (!logsPath.isEmpty())
    {
        ui->logsPathEdit->setText(logsPath);
    }
    m_logsPathChanged = true;
    emit optionChanged();
    emit restartRequired();
}

void ApplicationSettings::on_maxLogFileSpinBox_valueChanged([[maybe_unused]] int arg1)
{
    m_mLogSizeChanged = true;
    emit optionChanged();
}


void ApplicationSettings::on_showTrayBox_clicked()
{
    m_showTrayChanged = true;

    bool checked = ui->showTrayBox->isChecked();
    if (!checked)
    {
        ui->enaleNotifBox->setChecked(false);
        on_enaleNotifBox_clicked(false);
        ui->exitBehBtn->setCurrentIndex(SettingsValues::DESKTOP_EXIT_BEH_CLOSE);
        on_exitBehBtn_currentIndexChanged(SettingsValues::DESKTOP_EXIT_BEH_CLOSE);
    }
}

