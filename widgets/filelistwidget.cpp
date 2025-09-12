#include "filelistwidget.h"
#include "ui_filelistwidget.h"
#include "sessionmanager.h"

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

    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::STATUS), &m_statusDelegate);
    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::PROGRESS), &m_itemDelegate);

    connect(&m_fileModel, &FileTableModel::statusChanged, this, [&](int index, bool value) {
        auto& sessionManager = SessionManager::instance();
        auto currentTorrentOpt = sessionManager.getCurrentTorrentId();
        if (currentTorrentOpt.has_value()) {
            sessionManager.changeFilePriority(currentTorrentOpt.value(), index, value ? lt::default_priority : lt::dont_download);
        }
    });

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
