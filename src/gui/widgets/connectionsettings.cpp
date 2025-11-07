#include "connectionsettings.h"
#include "ui_connectionsettings.h"
#include <QSettings>
#include "gui/dialogs/managepeersdialog.h"
#include "core/utils/settingsvalues.h"
#include "core/controllers/sessionmanager.h"
#include <QMessageBox>

ConnectionSettings::ConnectionSettings(QWidget *parent)
    : BaseSettings(parent), ui(new Ui::ConnectionSettings)
{
    ui->setupUi(this);

    QSettings settings;
    {
        int port =
            settings.value(SettingsNames::LISTENING_PORT, SettingsValues::LISTENING_PORT_DEFAULT)
                .toInt();
        QSignalBlocker blocker{ui->portBox};
        ui->portBox->setValue(port);
    }
    {
        int protocolType = settings
                               .value(SettingsNames::LISTENING_PROTOCOL,
                                      SettingsValues::LISTENING_PROTOCOL_TCP_AND_UTP)
                               .toInt();
        QSignalBlocker blocker{ui->peerConnProtocolBox};
        ui->peerConnProtocolBox->setCurrentIndex(protocolType);
    }
    {
        int maxNumOfCon = settings
                              .value(SettingsNames::LIMITS_MAX_NUM_OF_CONNECTIONS,
                                     SettingsValues::LIMITS_MAX_NUM_OF_CONNECTIONS_DEFAULT)
                              .toInt();
        QSignalBlocker blocker{ui->mNumOfConBox};
        ui->mNumOfConBox->setValue(maxNumOfCon);
    }

    {
        int maxNumOfConPT = settings
                                .value(SettingsNames::LIMITS_MAX_NUM_OF_CONNECTIONS_PT,
                                       SettingsValues::LIMITS_MAX_NUM_OF_CONNECTIONS_PT_DEFAULT)
                                .toInt();
        QSignalBlocker blocker{ui->mNumOfConPTBox};
        ui->mNumOfConPTBox->setValue(maxNumOfConPT);
    }

    {
        bool upnpEnabled =
            settings.value(SettingsNames::LISTENING_UPNP, SettingsValues::LISTENING_UPNP_DEFAULT)
                .toBool();
        QSignalBlocker blocker{ui->natBox};
        ui->natBox->setChecked(upnpEnabled);
    }
}

ConnectionSettings::~ConnectionSettings() { delete ui; }

void ConnectionSettings::apply()
{
    auto     &sessionManager = SessionManager::instance();
    QSettings settings;
    if (m_portChanged)
    {
        int port = ui->portBox->value();
        settings.setValue(SettingsNames::LISTENING_PORT, port);
        sessionManager.setListenPort(port);

        m_portChanged = false;
    }
    if (m_protocolChanged)
    {

        int protocolType = ui->peerConnProtocolBox->currentIndex();
        settings.setValue(SettingsNames::LISTENING_PROTOCOL, protocolType);
        sessionManager.setListenProtocol(protocolType);

        m_protocolChanged = false;
    }
    if (m_mNumOfConChanged)
    {
        int value = ui->mNumOfConBox->value();
        settings.setValue(SettingsNames::LIMITS_MAX_NUM_OF_CONNECTIONS, value);
        sessionManager.setMaxNumberOfConnections(value);

        m_mNumOfConChanged = false;
    }
    if (m_mNumOfConPTChanged)
    {
        int value = ui->mNumOfConPTBox->value();
        settings.setValue(SettingsNames::LIMITS_MAX_NUM_OF_CONNECTIONS_PT, value);

        m_mNumOfConPTChanged = false;
    }
    if (m_upnpChanged)
    {
        bool value = ui->natBox->isChecked();
        settings.setValue(SettingsNames::LISTENING_UPNP, value);
        sessionManager.setUpnp(value);

        m_upnpChanged = false;
    }
}

void ConnectionSettings::on_portBox_valueChanged([[maybe_unused]] int arg1)
{
    m_portChanged = true;
    emit optionChanged();
}

void ConnectionSettings::on_resetPortBtn_clicked()
{
    ui->portBox->setValue(SettingsValues::LISTENING_PORT_DEFAULT);
    on_portBox_valueChanged(SettingsValues::LISTENING_PORT_DEFAULT);
}

void ConnectionSettings::on_peerConnProtocolBox_currentIndexChanged([[maybe_unused]] int index)
{
    m_protocolChanged = true;
    emit optionChanged();
}

void ConnectionSettings::on_mNumOfConBox_valueChanged([[maybe_unused]] int arg1)
{
    m_mNumOfConChanged = true;
    emit optionChanged();
}

void ConnectionSettings::on_mNumOfConPTBox_valueChanged([[maybe_unused]] int arg1)
{
    m_mNumOfConPTChanged = true;
    emit optionChanged();
}

void ConnectionSettings::on_mNumOfUplBox_valueChanged([[maybe_unused]] int arg1) {}

void ConnectionSettings::on_mNumOfUplBox_2_valueChanged([[maybe_unused]] int arg1) {}

void ConnectionSettings::on_managePeersButton_clicked()
{
    try
    {
        ManagePeersDialog dialog(this);
        if (QDialog::Accepted == dialog.exec())
        {
            auto  addrs          = dialog.getBannedPeers();
            auto &sessionManager = SessionManager::instance();
            sessionManager.setIpFilter(addrs);
        }
    }
    catch (const std::exception &ex)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Could not parse addresses"));
        qWarning() << "Could not parse all addresses in ManagePeersDialog";
    }
}

void ConnectionSettings::on_natBox_clicked() { m_upnpChanged = true; }
