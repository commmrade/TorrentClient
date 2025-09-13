#include "filelistwidget.h"
#include "ui_filelistwidget.h"
#include "sessionmanager.h"
#include <QAction>
#include <QMenu>
#include <QInputDialog>

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
    ui->fileView->setItemDelegateForColumn(static_cast<int>(FileFields::PRIORITY), &m_priorityDelegate);

    connect(&m_fileModel, &FileTableModel::statusChanged, this, [&](int index, bool value) {
        auto& sessionManager = SessionManager::instance();
        auto currentTorrentOpt = sessionManager.getCurrentTorrentId();
        if (currentTorrentOpt.has_value()) {
            sessionManager.changeFilePriority(currentTorrentOpt.value(), index, value ? lt::default_priority : lt::dont_download);
        }
    });
    connect(&m_fileModel, &FileTableModel::priorityChanged, this, [&](int index, int priority) {
        auto& sessionManager = SessionManager::instance();
        auto currentIdOpt = sessionManager.getCurrentTorrentId();
        if (currentIdOpt.has_value()) {
            qDebug() << "session mnager change priority" << priority;
            sessionManager.changeFilePriority(currentIdOpt.value(), index, priority);
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
    // auto index = ui->torrentsView->indexAt(pos);
    auto index = ui->fileView->indexAt(pos);
    if (index.row() == -1) return; /// indexAt() returns -1 when out of bounds

    auto fileId = m_fileModel.getFileId(index.row());
    auto& sessionManager = SessionManager::instance();
    QMenu mainMenu(this);
    QAction* renameAction = new QAction("Rename", this);
    connect(renameAction, &QAction::triggered, this, [this, &sessionManager, fileId] {\
        bool ok{};
        QString text = QInputDialog::getText(this, tr("Renaming"),
                                             tr("New file name:"), QLineEdit::Normal,
                                             "", &ok);
        if (ok && !text.isEmpty()) {
            auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
            if (currentTorrentIdOpt.has_value()) {
                sessionManager.renameFile(*currentTorrentIdOpt, fileId, text);
            }
        }
    });
    mainMenu.addAction(renameAction);

    QMenu* priorityMenu = mainMenu.addMenu("Priority");
    QAction* dontDownloadPriority = new QAction("Don't download", this);
    connect(dontDownloadPriority, &QAction::triggered, this, [this, &sessionManager, fileId] {
        auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
        if (currentTorrentIdOpt.has_value()) {
            sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId, lt::dont_download);
        }
    });
    priorityMenu->addAction(dontDownloadPriority);

    QAction* defaultPriority = new QAction("Default", this);
    connect(defaultPriority, &QAction::triggered, this, [this, &sessionManager, fileId] {
        auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
        if (currentTorrentIdOpt.has_value()) {
            sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId, lt::default_priority);
        }
    });
    priorityMenu->addAction(defaultPriority);

    QAction* lowPriority = new QAction("Low", this);
    connect(lowPriority, &QAction::triggered, this, [this, &sessionManager, fileId] {
        auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
        if (currentTorrentIdOpt.has_value()) {
            sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId, lt::low_priority);
        }
    });
    priorityMenu->addAction(lowPriority);

    QAction* topPriority = new QAction("High", this);
    connect(topPriority, &QAction::triggered, this, [this, &sessionManager, fileId] {
        auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
        if (currentTorrentIdOpt.has_value()) {
            sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId, lt::top_priority);
        }
    });
    priorityMenu->addAction(topPriority);

    mainMenu.exec(ui->fileView->viewport()->mapToGlobal(pos));
}
