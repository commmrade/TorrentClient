#ifndef MANAGEPEERSDIALOG_H
#define MANAGEPEERSDIALOG_H

#include <QDialog>
#include <boost/asio.hpp>

namespace Ui
{
class ManagePeersDialog;
}

class ManagePeersDialog final : public QDialog
{
    Q_OBJECT

  public:
    explicit ManagePeersDialog(QWidget *parent = nullptr);
    ~ManagePeersDialog();

    QList<boost::asio::ip::address> getBannedPeers() const;

  private:
    Ui::ManagePeersDialog *ui;
};

#endif // MANAGEPEERSDIALOG_H
