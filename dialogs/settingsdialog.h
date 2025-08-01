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

private:
    Ui::SettingsDialog *ui;


    void applyApplicationSettings();
    void applyTorrentSettings();

signals:
    void downloadLimitChanged(int value);
    void uploadLimitChanged(int value);
};

#endif // SETTINGSDIALOG_H
