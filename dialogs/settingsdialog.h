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
    CONNECTION
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

    void on_portBox_valueChanged(int arg1);

    void on_resetPortBtn_clicked();

    void on_peerConnProtocolBox_currentIndexChanged(int index);

    void on_mNumOfConBox_valueChanged(int arg1);

    void on_mNumOfConPTBox_valueChanged(int arg1);

    void on_managePeersButton_clicked();

    void on_resetSesionButton_clicked();

    void on_dhtCheck_clicked();

    void on_peerExCheck_clicked();

    void on_localPeerDiscCheck_clicked();

  private:
    Ui::SettingsDialog *ui;

    // Settings flags
    bool m_restartRequired{false};
    /// General
    bool m_languageChanged{false};
    bool m_themeChanged{false};
    bool m_confirmDeleteChanged{false};
    bool m_resetChanged{false};
    // Tray
    bool m_showTrayChanged{false};
    bool m_enableNotifChanged{false};
    bool m_exitBehChanged{false};
    // Logs
    bool m_logsEnabledChanged{false};
    bool m_mLogSizeChanged{false};
    bool m_logsPathChanged{false};

    /// Torrent
    bool m_downloadLimitChanged{false};
    bool m_uploadLimitChanged{false};
    bool m_savePathChanged{false};
    bool m_dhtChanged{false};
    bool m_peerExChanged{false};
    bool m_localPeerDiscChanged{false};

    /// Connection
    bool m_portChanged{false};
    bool m_protocolChanged{false};
    bool m_mNumOfConChanged{false};
    bool m_mNumOfConPTChanged{false};


    void applyApplicationSettings();
    void applyTorrentSettings();
    void applyConnectionSettings();

  signals:
    void downloadLimitChanged(int value);
    void uploadLimitChanged(int value);
};

#endif // SETTINGSDIALOG_H
