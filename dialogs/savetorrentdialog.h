#ifndef SAVETORRENTDIALOG_H
#define SAVETORRENTDIALOG_H

#include <QDialog>
#include "metadatafetcher.h"
#include <QPointer>

namespace Ui {
class SaveTorrentDialog;
}

struct torrent_file_tag {};
struct magnet_tag {};

class SaveTorrentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SaveTorrentDialog(torrent_file_tag, const QString& torrentPath, QWidget *parent = nullptr); // for .torrent
    explicit SaveTorrentDialog(magnet_tag, const QString& magnetUri, QWidget* parent = nullptr);
    ~SaveTorrentDialog();
    QString getSavePath() const;

public slots:
    void setSize(std::int64_t bytes);
private slots:
    void on_changeSavePathButton_clicked();
private:
    Ui::SaveTorrentDialog *ui;
    QPointer<MetadataFetcher> m_fetcher{nullptr};

    void tryLoadData();
};

#endif // SAVETORRENTDIALOG_H
