#include "managepeersdialog.h"
#include "ui_managepeersdialog.h"
#include "sessionmanager.h"
#include "boost/asio.hpp"
#include <QMessageBox>

ManagePeersDialog::ManagePeersDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ManagePeersDialog)
{
    ui->setupUi(this);

    auto &sessionManager = SessionManager::instance();
    auto  peersTuple     = sessionManager.getIpFilter();

    auto bPeersV4 = std::get<0>(peersTuple);
    auto bPeersV6 = std::get<1>(peersTuple);

    for (int i = 0; i < (int)bPeersV4.size(); ++i)
    {
        if (bPeersV4[i].flags != lt::ip_filter::blocked)
        {
            continue;
        }

        uint start = bPeersV4[i].first.to_uint();
        uint end   = bPeersV4[i].last.to_uint();
        for (uint i = start; i <= end; ++i)
        {
            boost::asio::ip::address addr = boost::asio::ip::make_address_v4(i);
            ui->textEdit->append(QString::fromStdString(addr.to_string()));
        }
    }
    for (int i = 0; i < (int)bPeersV6.size(); ++i)
    {
        if (bPeersV6[i].flags != lt::ip_filter::blocked)
        {
            continue;
        }
        boost::asio::ip::address addr{bPeersV6[i].first};
        ui->textEdit->append(QString::fromStdString(addr.to_string()));
    }
}

ManagePeersDialog::~ManagePeersDialog() { delete ui; }

QList<boost::asio::ip::address> ManagePeersDialog::getBannedPeers() const
{
    auto        peers = ui->textEdit->toPlainText();
    QTextStream stream(&peers);

    QList<boost::asio::ip::address> m_peers;
    QString                         line;
    while (stream.readLineInto(&line))
    {
        if (line.isEmpty())
        {
            continue;
        }
        auto addr = boost::asio::ip::make_address(line.toStdString());
        m_peers.append(std::move(addr));
    }
    return m_peers;
}
