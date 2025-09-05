#include "addpeersdialog.h"
#include "ui_addpeersdialog.h"
#include <QCloseEvent>
// #include <boost/asio/ip/tcp.hpp>
#include <QMessageBox>

AddPeersDialog::AddPeersDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddPeersDialog)
{
    ui->setupUi(this);
}

AddPeersDialog::~AddPeersDialog()
{
    delete ui;
}

bool AddPeersDialog::parseEndpoints()
{
    auto endpointsStr = ui->textEdit->toPlainText();
    QTextStream stream(&endpointsStr);

    QString line;
    QList<boost::asio::ip::tcp::endpoint> eps;
    while (stream.readLineInto(&line)) {
        auto addrPortStr = line.toUtf8();
        auto addrPortView = QByteArrayView{addrPortStr};
        auto colIndexIter = std::find_if(addrPortView.begin(), addrPortView.end(), [](const auto ch) {
            return ch == ':';
        });
        if (colIndexIter == addrPortView.end()) {
            return false;
        }
        auto colIndex = colIndexIter - addrPortView.begin();
        auto addrStr = addrPortView.sliced(0, colIndex);
        auto portStr = addrPortView.sliced(colIndex + 1, addrPortView.size() - (colIndex + 1));
        qDebug() << addrStr << portStr;

        boost::system::error_code ec;
        auto addr = boost::asio::ip::make_address(addrStr.toByteArray().constData(), ec);
        if (ec) {
            qDebug() << "False here";
            return false;
        }
        auto ep = boost::asio::ip::tcp::endpoint{addr, static_cast<unsigned short>(portStr.toInt())};
        eps.append(std::move(ep));
    }
    m_addrs = std::move(eps);
    return true;
}

void AddPeersDialog::done(int a)
{
    if (a != QDialog::Rejected) { // not cancel pressed or esc
        if (!parseEndpoints()) {
            QMessageBox::warning(this, "Warning", "Could not parse all addresses, make sure they are correct");
            return;
        }
    }
    QDialog::done(a);
}
