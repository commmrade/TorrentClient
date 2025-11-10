#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include "basesettings.h"

namespace Ui
{
class ApplicationSettings;
}

class ApplicationSettings final : public BaseSettings
{
    Q_OBJECT
  public:
    explicit ApplicationSettings(QWidget *parent = nullptr);
    ~ApplicationSettings();

    void apply() override;
    void setupFields() override;
  private slots:
    void on_languageBox_currentIndexChanged(int index);

    void on_themeBox_currentIndexChanged(int index);

    void on_chooseThemeBtn_clicked();

    void on_confirmDelBox_clicked(bool checked);

    void on_enaleNotifBox_clicked(bool checked);

    void on_exitBehBtn_currentIndexChanged(int index);

    void on_logsBox_clicked(bool checked);

    void on_logsPathBtn_clicked();

    void on_maxLogFileSpinBox_valueChanged(int arg1);

    void on_showTrayBox_clicked();

  private:
    Ui::ApplicationSettings *ui;

    bool m_languageChanged{false};
    bool m_themeChanged{false};
    bool m_confirmDeleteChanged{false};
    bool m_logsEnabledChanged{false};
    bool m_showTrayChanged{false};
    bool m_enableNotifChanged{false};
    bool m_exitBehChanged{false};
    bool m_mLogSizeChanged{false};
    bool m_logsPathChanged{false};
};

#endif // APPLICATIONSETTINGS_H
