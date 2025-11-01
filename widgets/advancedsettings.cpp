#include "advancedsettings.h"
#include "ui_advancedsettings.h"
#include <QSettings>
#include "settingsvalues.h"
#include "sessionmanager.h"

AdvancedSettings::AdvancedSettings(QWidget *parent) : BaseSettings(parent), ui(new Ui::AdvancedSettings)
{
    ui->setupUi(this);

    QSettings settings;

    int torRmMode = settings.value(SettingsNames::ADVANCED_TORRENT_REMOVE_MODE, SettingsValues::ADVANCED_TORRENT_REMOVE_MODE_DELETE).toInt();
    QSignalBlocker torRmB{ui->torrentRmModeBox};
    ui->torrentRmModeBox->setCurrentIndex(torRmMode);

    int loopDur = settings.value(SettingsNames::ADVANCED_LOOP_DURATION, SettingsValues::ADVANCED_LOOP_DURATION).toInt();
    QSignalBlocker loopB{ui->loopDurBox};
    ui->loopDurBox->setValue(loopDur);
}

AdvancedSettings::~AdvancedSettings() { delete ui; }

void AdvancedSettings::apply()
{
    QSettings settings;
    auto& sessionManager = SessionManager::instance();
    if (m_torrentRmModeChanged) {
        int value = ui->torrentRmModeBox->currentIndex();
        settings.setValue(SettingsNames::ADVANCED_TORRENT_REMOVE_MODE, value);

        m_torrentRmModeChanged = false;
    }
    if (m_loopDurChanged) {
        int value = ui->loopDurBox->value();
        settings.setValue(SettingsNames::ADVANCED_LOOP_DURATION, value);
        sessionManager.setLoopDuration(value);
        m_loopDurChanged = false;
    }
}

void AdvancedSettings::on_torrentRmModeBox_currentIndexChanged([[maybe_unused]] int index)
{
    m_torrentRmModeChanged = true;
    emit optionChanged();
}


void AdvancedSettings::on_loopDurBox_valueChanged([[maybe_unused]] int arg1)
{
    m_loopDurChanged = true;
    emit optionChanged();
}

