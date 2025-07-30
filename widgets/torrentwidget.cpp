#include "torrentwidget.h"
#include "ui_torrentwidget.h"
#include <QFileDialog>
#include <QTableWidgetItem>

TorrentWidget::TorrentWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TorrentWidget)
{
    ui->setupUi(this);

    m_tableModel = new TorrentsTableModel{parent};
    m_tableDelegate = new TorrentItemDelegate{parent};

    ui->tableView->setModel(m_tableModel);
    ui->tableView->setItemDelegateForColumn(3, m_tableDelegate);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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

