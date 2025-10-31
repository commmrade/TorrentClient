#ifndef CONNECTIONSETTINGS_H
#define CONNECTIONSETTINGS_H

#include "basesettings.h"

namespace Ui
{
class ConnectionSettings;
}

class ConnectionSettings : public BaseSettings
{
    Q_OBJECT

  public:
    explicit ConnectionSettings(QWidget *parent = nullptr);
    ~ConnectionSettings();

    void apply() override;
  private slots:
    void on_portBox_valueChanged(int arg1);

    void on_resetPortBtn_clicked();

    void on_peerConnProtocolBox_currentIndexChanged(int index);

    void on_mNumOfConBox_valueChanged(int arg1);

    void on_mNumOfConPTBox_valueChanged(int arg1);

    void on_mNumOfUplBox_valueChanged(int arg1);

    void on_mNumOfUplBox_2_valueChanged(int arg1);

    void on_managePeersButton_clicked();

  private:
    Ui::ConnectionSettings *ui;

    bool m_portChanged{false};
    bool m_protocolChanged{false};
    bool m_mNumOfConChanged{false};
    bool m_mNumOfConPTChanged{false};
};

#endif // CONNECTIONSETTINGS_H
