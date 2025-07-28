#include "torrentwidget.h"
#include "ui_torrentwidget.h"
#include <QFileDialog>

TorrentWidget::TorrentWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TorrentWidget)
{
    ui->setupUi(this);
}

TorrentWidget::~TorrentWidget()
{
    delete ui;
}

void TorrentWidget::on_pushButton_clicked()
{

    m_sessionManager.addTorrentByMagnet(ui->lineEdit->text());
    qDebug() << "Torrent added";
}


void TorrentWidget::on_pushButton_2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "OPen torrent", "/home/klewy", "Torrents (*.torrent)");
    if (!filename.isEmpty())
        m_sessionManager.addTorrentByFilename(filename);
}

