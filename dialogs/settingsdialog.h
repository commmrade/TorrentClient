#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}


enum SettingsTabs {
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

    void on_customizeButton_clicked();

    void on_applyButton_clicked();

    void on_savePathButton_clicked();

    void on_languageBox_currentTextChanged(const QString &arg1);

    void on_themeBox_currentTextChanged(const QString &arg1);

    void on_downloadLimitSpin_valueChanged(int arg1);

    void on_uploadLimitSpin_valueChanged(int arg1);

    void on_okButton_clicked();

    void on_closeButton_clicked();

private:
    Ui::SettingsDialog *ui;

    // Settings flags
    bool m_restartRequired{false};
    // General
    bool m_languageChanged{false};
    bool m_themeChanged{false};
    // Torrent
    bool m_downloadLimitChanged{false};
    bool m_uploadLimitChanged{false};

    void applyApplicationSettings();
    void applyTorrentSettings();

signals:
    void downloadLimitChanged(int value);
    void uploadLimitChanged(int value);
};

#endif // SETTINGSDIALOG_H
