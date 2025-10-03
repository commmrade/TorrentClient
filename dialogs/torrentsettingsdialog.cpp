#include "torrentsettingsdialog.h"
#include "ui_torrentsettingsdialog.h"
#include <QFileDialog>

TorrentSettingsDialog::TorrentSettingsDialog(const TorrentHandle &tHandle, QWidget *parent)
    : QDialog(parent), ui(new Ui::TorrentSettingsDialog)
{
    ui->setupUi(this);

    auto downloadLimitBytes = tHandle.handle().download_limit();
    auto downloadLimitKB = downloadLimitBytes / 1024.0;
    auto uploadLimitBytes = tHandle.handle().upload_limit();
    auto uploadLimitKB = uploadLimitBytes / 1024.0;

    auto status = tHandle.handle().status(lt::torrent_handle::query_save_path);
    QString savePath = QString::fromStdString(status.save_path);
    ui->savePathLineEdit->setText(savePath);
    ui->downloadLimitSpin->setValue(downloadLimitKB);
    ui->uploadLimitSpin->setValue(uploadLimitKB);

    connect(this, &QDialog::finished, this, [this](int result) {
        if (result == QDialog::Accepted) {
            applySettings();
        }
    });
    connect(ui->savePathLineEdit, &QLineEdit::textChanged, this, [this] {
        m_saveLocChanged = true;
    });
}

TorrentSettingsDialog::~TorrentSettingsDialog() { delete ui; }

void TorrentSettingsDialog::applySettings()
{
    if (m_downloadLimitChanged) {
        auto downloadLimitBytes = ui->downloadLimitSpin->value() * 1024;
        emit downloadLimitChanged(downloadLimitBytes);
    }
    if (m_uploadLimitChanged) {
        auto uploadLimitBytes = ui->uploadLimitSpin->value() * 1024;
        emit uploadLimitChanged(uploadLimitBytes);
    }
    if (m_saveLocChanged) {
        auto newPath = ui->savePathLineEdit->text();
        emit savePathChanged(newPath);
    }
}


void TorrentSettingsDialog::on_downloadLimitSpin_valueChanged(int arg1)
{
    m_downloadLimitChanged = true;
}


void TorrentSettingsDialog::on_uploadLimitSpin_valueChanged(int arg1)
{
    m_uploadLimitChanged = true;
}


void TorrentSettingsDialog::on_savePathButton_clicked()
{
    QString saveDir =
        QFileDialog::getExistingDirectory(this, tr("Choose a directory to move storage to"));
    if (!saveDir.isEmpty())
    {
        ui->savePathLineEdit->setText(saveDir);
        m_saveLocChanged = true;
    }
}

