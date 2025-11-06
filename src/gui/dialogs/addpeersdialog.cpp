#include "gui/dialogs/addpeersdialog.h"
#include "ui_addpeersdialog.h"
#include <QCloseEvent>
#include <QMessageBox>

AddPeersDialog::AddPeersDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AddPeersDialog)
{
    ui->setupUi(this);
}

AddPeersDialog::~AddPeersDialog() { delete ui; }

QList<boost::asio::ip::tcp::endpoint> AddPeersDialog::parseEndpoints()
{
    QList<boost::asio::ip::tcp::endpoint> eps;

    auto        endpointsStr = ui->textEdit->toPlainText();
    QTextStream stream(&endpointsStr);

    QString line;
    while (stream.readLineInto(&line))
    {
        auto addrPortStr  = line.toUtf8();
        auto addrPortView = QByteArrayView{addrPortStr};
        auto colIndexIter = std::find_if(addrPortView.begin(), addrPortView.end(),
                                         [](const auto ch) { return ch == ':'; });
        if (colIndexIter == addrPortView.end())
        {
            throw std::runtime_error("Could not parse endpoints");
        }
        auto colIndex = colIndexIter - addrPortView.begin();
        auto addrStr  = addrPortView.sliced(0, colIndex);
        auto portStr  = addrPortView.sliced(colIndex + 1, addrPortView.size() - (colIndex + 1));

        auto addr = boost::asio::ip::make_address(addrStr.toByteArray().constData());
        auto ep =
            boost::asio::ip::tcp::endpoint{addr, static_cast<unsigned short>(portStr.toInt())};
        eps.append(std::move(ep));
    }
    return eps;
}
