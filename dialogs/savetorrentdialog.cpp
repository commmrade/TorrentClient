#include "savetorrentdialog.h"
#include "ui_savetorrentdialog.h"
#include <libtorrent/torrent_info.hpp>
#include <QSettings>
#include "settingsvalues.h"
#include <QStandardPaths>

SaveTorrentDialog::SaveTorrentDialog(const QString& torrentPath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SaveTorrentDialog)
{
    ui->setupUi(this);

    lt::torrent_info info{torrentPath.toStdString()};
    auto totalSize = info.total_size() / 1024.0 / 1024.0;
    ui->sizeInfo->setText(QString::number(totalSize) + " MB");

    QSettings settings;
    auto savePath = settings.value(SettingsValues::SESSION_DEFAULT_SAVE_LOCATION, QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    ui->savePathLineEdit->setText(savePath);
}

SaveTorrentDialog::SaveTorrentDialog(int i, const QString &magnetUri, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SaveTorrentDialog)
{
    ui->setupUi(this);

    ui->sizeInfo->setText("Not known");
    QSettings settings;
    auto savePath = settings.value(SettingsValues::SESSION_DEFAULT_SAVE_LOCATION, QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    ui->savePathLineEdit->setText(savePath);
}

SaveTorrentDialog::~SaveTorrentDialog()
{
    delete ui;
}

QString SaveTorrentDialog::getSavePath() const
{
    return ui->savePathLineEdit->text();
}

void SaveTorrentDialog::on_changeSavePathButton_clicked()
{
    // TODO: Change Dir
    ui->savePathLineEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
}

