#ifndef TORRENTSETTINGS_H
#define TORRENTSETTINGS_H

#include "basesettings.h"

namespace Ui
{
class TorrentSettings;
}

class TorrentSettings : public BaseSettings
{
    Q_OBJECT

  public:
    explicit TorrentSettings(QWidget *parent = nullptr);
    ~TorrentSettings();

    void apply() override;
  private slots:
    void on_downloadLimitSpin_valueChanged(int arg1);

    void on_uploadLimitSpin_valueChanged(int arg1);

    void on_resetSessionButton_clicked();

    void on_savePathButton_clicked();

    void on_dhtCheck_clicked(bool checked);

    void on_peerExCheck_clicked(bool checked);

    void on_localPeerDiscCheck_clicked(bool checked);

  private:
    Ui::TorrentSettings *ui;

    bool m_downloadLimitChanged{false};
    bool m_uploadLimitChanged{false};
    bool m_savePathChanged{false};
    bool m_resetChanged{false};
    bool m_dhtChanged{false};
    bool m_peerExChanged{false};
    bool m_localPeerDiscChanged{false};
};

#endif // TORRENTSETTINGS_H
