#ifndef DELETETORRENTDIALOG_H
#define DELETETORRENTDIALOG_H

#include <QDialog>

namespace Ui
{
class DeleteTorrentDialog;
}

class DeleteTorrentDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit DeleteTorrentDialog(QWidget *parent = nullptr);
    ~DeleteTorrentDialog();

    bool getRemoveWithContens() const;

  private:
    Ui::DeleteTorrentDialog *ui;
};

#endif // DELETETORRENTDIALOG_H
