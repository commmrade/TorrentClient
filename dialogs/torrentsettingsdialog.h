#ifndef TORRENTSETTINGSDIALOG_H
#define TORRENTSETTINGSDIALOG_H

#include <QDialog>
#include "torrenthandle.h"

namespace Ui
{
class TorrentSettingsDialog;
}

class TorrentSettingsDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit TorrentSettingsDialog(const TorrentHandle &tHandle, QWidget *parent = nullptr);
    ~TorrentSettingsDialog();

  signals:
    void downloadLimitChanged(int newLimit);
    void uploadLimitChanged(int newLimit);
    void savePathChanged(const QString &newPath);
    void maxNumOfConChanged(int newValue);
    void dhtChanged(bool value);
    void pexChanged(bool value);
    void lsdChanged(bool value);
  private slots:
    void on_downloadLimitSpin_valueChanged(int arg1);

    void on_uploadLimitSpin_valueChanged(int arg1);

    void on_savePathButton_clicked();

    void on_maxNumOfConBox_valueChanged(int arg1);
    void on_dhtCheckBox_clicked();

    void on_pexCheckBox_clicked();

    void on_lpdCheckBox_clicked();

  private:
    Ui::TorrentSettingsDialog *ui;

    void applySettings();

    bool m_uploadLimitChanged{false};
    bool m_downloadLimitChanged{false};
    bool m_saveLocChanged{false};
    bool m_mNumOfConChanged{false};
    bool m_dhtChanged{false};
    bool m_pexChanged{false};
    bool m_lsdChanged{false};
};

#endif // TORRENTSETTINGSDIALOG_H
