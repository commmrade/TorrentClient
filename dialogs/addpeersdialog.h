#ifndef ADDPEERSDIALOG_H
#define ADDPEERSDIALOG_H

#include <QDialog>
#include <boost/asio.hpp>

namespace Ui {
class AddPeersDialog;
}


class AddPeersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddPeersDialog(QWidget *parent = nullptr);
    ~AddPeersDialog();

    bool parseEndpoints();
    void done(int) override;

    // Usable only 1 time, since addresses are moved
    QList<boost::asio::ip::tcp::endpoint> getAddrs() {
        return std::move(m_addrs);
    }
private:
    Ui::AddPeersDialog *ui;

    QList<boost::asio::ip::tcp::endpoint> m_addrs;
};

#endif // ADDPEERSDIALOG_H
