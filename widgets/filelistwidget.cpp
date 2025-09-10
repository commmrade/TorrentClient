#include "filelistwidget.h"
#include "ui_filelistwidget.h"

FileListWidget::FileListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileListWidget)
{
    ui->setupUi(this);

    ui->fileView->setModel(&m_fileModel);
    ui->fileView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->fileView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->fileView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->fileView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::PROGRESS), &m_itemDelegate);

    connect(ui->fileView, &QTableView::customContextMenuRequested, this, &FileListWidget::contextMenuRequested);
}

FileListWidget::~FileListWidget()
{
    delete ui;
}

void FileListWidget::setFiles(const QList<File> &files)
{
    m_fileModel.setFiles(files);
}

void FileListWidget::clearFiles()
{
    m_fileModel.clearFiles();
}

void FileListWidget::contextMenuRequested(const QPoint &pos)
{
    // TODO: Rename, change priority
}
