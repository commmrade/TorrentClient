#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "applicationsettings.h"
#include "connectionsettings.h"
#include "torrentsettings.h"
#include <QPointer>
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
    void onRestartRequired();
    void onOptionChanged();
    void on_applyButton_clicked();
    void on_okButton_clicked();

    void on_closeButton_clicked();

  private:
    Ui::SettingsDialog *ui;

    bool m_restartRequired{false};
    bool m_optionChanged{false};

    QPointer<ApplicationSettings> appSettings;
    QPointer<TorrentSettings>     torSettings;
    QPointer<ConnectionSettings>  connSettings;
};

#endif // SETTINGSDIALOG_H
