#ifndef ADDPEERSDIALOG_H
#define ADDPEERSDIALOG_H

#include <QDialog>
#include <boost/asio.hpp>

namespace Ui
{
class AddPeersDialog;
}

class AddPeersDialog final : public QDialog
{
    Q_OBJECT

  public:
    explicit AddPeersDialog(QWidget *parent = nullptr);
    ~AddPeersDialog();

    QList<boost::asio::ip::tcp::endpoint> parseEndpoints();

  private:
    Ui::AddPeersDialog *ui;
};

#endif // ADDPEERSDIALOG_H
