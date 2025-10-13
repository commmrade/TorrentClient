#ifndef LOADMAGNETDIALOG_H
#define LOADMAGNETDIALOG_H

#include <QDialog>
#include <libtorrent/magnet_uri.hpp>

namespace Ui
{
class LoadMagnetDialog;
}

class LoadMagnetDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit LoadMagnetDialog(QWidget *parent = nullptr);
    ~LoadMagnetDialog();

    QList<QString> getMagnets() const;
  private:
    Ui::LoadMagnetDialog *ui;
};

#endif // LOADMAGNETDIALOG_H
