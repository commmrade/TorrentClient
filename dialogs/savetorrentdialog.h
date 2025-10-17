#ifndef SAVETORRENTDIALOG_H
#define SAVETORRENTDIALOG_H

#include <QDialog>
#include "metadatafetcher.h"
#include <QPointer>
#include <fileitemdelegate.h>
#include <fileprioritydelegate.h>
#include <filestatusdelegate.h>
#include "filetreemodel.h"

namespace Ui
{
class SaveTorrentDialog;
}

struct torrent_file_tag
{
};
struct magnet_tag
{
};

class SaveTorrentDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit SaveTorrentDialog(torrent_file_tag, const QString &torrentPath,
                               QWidget *parent = nullptr); // for .torrent
    explicit SaveTorrentDialog(magnet_tag, const QString &magnetUri, QWidget *parent = nullptr);
    ~SaveTorrentDialog();
    QString getSavePath() const;
    std::shared_ptr<const lt::torrent_info> getTorrentInfo() const;
    QList<lt::download_priority_t> getFilePriorities() const;

    void setData(std::shared_ptr<const lt::torrent_info> ti);
  private slots:
    void on_changeSavePathButton_clicked();

  private:
    Ui::SaveTorrentDialog *ui;

    std::shared_ptr<const lt::torrent_info> m_torrentInfo{nullptr};
    QList<lt::download_priority_t>          m_filePriorities;

    FileTreeModel        m_fileModel;
    FileStatusDelegate   m_statusDelegate;
    FileItemDelegate     m_itemDelegate;
    FilePriorityDelegate m_priorityDelegate;

    void startFetchingMetadata(const lt::add_torrent_params &params);
    void setDataFromTi();

    void setupTableView();

    void init();
};

#endif // SAVETORRENTDIALOG_H
