#ifndef SAVETORRENTDIALOG_H
#define SAVETORRENTDIALOG_H

#include <QDialog>

namespace Ui {
class SaveTorrentDialog;
}

class SaveTorrentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SaveTorrentDialog(const QString& torrentPath, QWidget *parent = nullptr); // for .torrent
    explicit SaveTorrentDialog(int i, const QString& magnetUri, QWidget* parent = nullptr);
    ~SaveTorrentDialog();
    QString getSavePath() const;
private slots:
    void on_changeSavePathButton_clicked();

private:
    Ui::SaveTorrentDialog *ui;
};

#endif // SAVETORRENTDIALOG_H
