#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui
{
class SettingsDialog;
}

enum SettingsTabs
{
    APPLICATION,
    TORRENT,
    ADVANCED
};

class SettingsDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

  private slots:
    void on_listWidget_currentRowChanged(int currentRow);

    // void on_customizeButton_clicked();

    void on_applyButton_clicked();

    void on_savePathButton_clicked();

    void on_downloadLimitSpin_valueChanged(int arg1);

    void on_uploadLimitSpin_valueChanged(int arg1);

    void on_okButton_clicked();

    void on_closeButton_clicked();

    void on_chooseThemeBtn_clicked();

    void on_confirmDelBox_clicked(bool checked);
    void on_showTrayBox_clicked(bool checked);

    void on_enaleNotifBox_clicked(bool checked);

    void on_exitBehBtn_currentIndexChanged(int index);

    void on_languageBox_currentIndexChanged(int index);

    void on_themeBox_currentIndexChanged(int index);

    void on_logsPathBtn_clicked();

    void on_maxLogFileSpinBox_valueChanged(int arg1);

    void on_logsBox_clicked(bool checked);

  private:
    Ui::SettingsDialog *ui;

    // Settings flags
    bool m_restartRequired{false};
    // General
    bool m_languageChanged{false};
    bool m_themeChanged{false};
    bool m_confirmDeleteChanged{false};

    // Torrent
    bool m_downloadLimitChanged{false};
    bool m_uploadLimitChanged{false};
    bool m_savePathChanged{false};

    // Tray
    bool m_showTrayChanged{false};
    bool m_enableNotifChanged{false};
    bool m_exitBehChanged{false};

    // Logs
    bool m_logsEnabledChanged{false};
    bool m_mLogSizeChanged{false};
    bool m_logsPathChanged{false};

    void applyApplicationSettings();
    void applyTorrentSettings();

  signals:
    void downloadLimitChanged(int value);
    void uploadLimitChanged(int value);
};

#endif // SETTINGSDIALOG_H
