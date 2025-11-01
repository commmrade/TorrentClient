#ifndef ADVANCEDSETTINGS_H
#define ADVANCEDSETTINGS_H

#include "basesettings.h"

namespace Ui
{
class AdvancedSettings;
}

class AdvancedSettings : public BaseSettings
{
    Q_OBJECT
  public:
    explicit AdvancedSettings(QWidget *parent = nullptr);
    ~AdvancedSettings();

    void apply() override;

  private slots:
    void on_torrentRmModeBox_currentIndexChanged(int index);

    void on_loopDurBox_valueChanged(int arg1);

  private:
    Ui::AdvancedSettings *ui;
    bool m_torrentRmModeChanged{false};
    bool m_loopDurChanged{false};
};

#endif // ADVANCEDSETTINGS_H
