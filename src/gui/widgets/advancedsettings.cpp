#include "advancedsettings.h"
#include "ui_advancedsettings.h"
#include <QSettings>
#include "core/utils/settingsvalues.h"
#include "core/controllers/sessionmanager.h"

AdvancedSettings::AdvancedSettings(QWidget *parent) : BaseSettings(parent), ui(new Ui::AdvancedSettings)
{
    ui->setupUi(this);

    QSettings settings;

    int loopDur = settings.value(SettingsNames::ADVANCED_LOOP_DURATION, SettingsValues::ADVANCED_LOOP_DURATION).toInt();
    QSignalBlocker loopB{ui->loopDurBox};
    ui->loopDurBox->setValue(loopDur);
}

AdvancedSettings::~AdvancedSettings() { delete ui; }

void AdvancedSettings::apply()
{
    QSettings settings;
    auto& sessionManager = SessionManager::instance();
    if (m_loopDurChanged) {
        int value = ui->loopDurBox->value();
        settings.setValue(SettingsNames::ADVANCED_LOOP_DURATION, value);
        sessionManager.setLoopDuration(value);
        m_loopDurChanged = false;
    }
}

void AdvancedSettings::on_loopDurBox_valueChanged([[maybe_unused]] int arg1)
{
    m_loopDurChanged = true;
    emit optionChanged();
}

