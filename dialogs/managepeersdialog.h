#ifndef MANAGEPEERSDIALOG_H
#define MANAGEPEERSDIALOG_H

#include <QDialog>
#include <boost/asio.hpp>

namespace Ui
{
class ManagePeersDialog;
}

class ManagePeersDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ManagePeersDialog(QWidget *parent = nullptr);
    ~ManagePeersDialog();

    QList<boost::asio::ip::address> getBannedPeers() const {
        return m_peers;
    }
  private slots:
    void on_buttonBox_accepted();


  private:
    Ui::ManagePeersDialog *ui;

    QList<boost::asio::ip::address> m_peers;
};

#endif // MANAGEPEERSDIALOG_H
