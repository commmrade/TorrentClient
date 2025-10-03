#include "filelistwidget.h"
#include "priority.h"
#include "ui_filelistwidget.h"
#include "sessionmanager.h"
#include <QAction>
#include <QMenu>
#include <QInputDialog>

FileListWidget::FileListWidget(QWidget *parent) : QWidget(parent), ui(new Ui::FileListWidget)
{
    ui->setupUi(this);

    setupTableView();

    connect(&m_fileModel, &FileTreeModel::statusChanged, this,
            [&](int index, bool value)
            {
                auto &sessionManager    = SessionManager::instance();
                auto  currentTorrentOpt = sessionManager.getCurrentTorrentId();
                if (currentTorrentOpt.has_value())
                {
                    sessionManager.changeFilePriority(currentTorrentOpt.value(), index,
                                                      value ? lt::default_priority
                                                            : lt::dont_download);
                }
            });
    connect(&m_fileModel, &FileTreeModel::priorityChanged, this,
            [&](int index, int priority)
            {
                auto &sessionManager = SessionManager::instance();
                auto  currentIdOpt   = sessionManager.getCurrentTorrentId();
                if (currentIdOpt.has_value())
                {
                    sessionManager.changeFilePriority(currentIdOpt.value(), index, priority);
                }
            });

    connect(ui->treeView, &QTreeView::customContextMenuRequested, this,
            &FileListWidget::contextMenuRequested);
}

FileListWidget::~FileListWidget() { delete ui; }

void FileListWidget::setFiles(const QList<File> &files) { m_fileModel.setFiles(files); }

void FileListWidget::clearFiles() { m_fileModel.clearFiles(); }

void FileListWidget::contextMenuRequested(const QPoint &pos)
{
    // TODO: Rename, change priority
    // auto index = ui->torrentsView->indexAt(pos);
    auto index = ui->treeView->indexAt(pos);
    if (index.row() == -1)
        return; /// indexAt() returns -1 when out of bounds

    // auto fileId = m_fileModel.getFileId(index.row());
    // auto& sessionManager = SessionManager::instance();
    // QMenu mainMenu(this);
    // QAction* renameAction = new QAction(tr("Rename"), this);
    // connect(renameAction, &QAction::triggered, this, [this, &sessionManager, fileId] {\
    //     bool ok{};
    //     QString text = QInputDialog::getText(this, tr("Renaming"),
    //                                          tr("New file name:"), QLineEdit::Normal,
    //                                          "", &ok);
    //     if (ok && !text.isEmpty()) {
    //         auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
    //         if (currentTorrentIdOpt.has_value()) {
    //             sessionManager.renameFile(*currentTorrentIdOpt, fileId, text);
    //         }
    //     }
    // });
    // mainMenu.addAction(renameAction);

    // QMenu* priorityMenu = mainMenu.addMenu(tr("Priority"));
    // QAction* dontDownloadPriority = new QAction(Priorities::DONT_DOWNLOAD, this);
    // connect(dontDownloadPriority, &QAction::triggered, this, [this, &sessionManager, fileId] {
    //     auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
    //     if (currentTorrentIdOpt.has_value()) {
    //         sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId, lt::dont_download);
    //     }
    // });
    // priorityMenu->addAction(dontDownloadPriority);

    // QAction* defaultPriority = new QAction(Priorities::DEFAULT, this);
    // connect(defaultPriority, &QAction::triggered, this, [this, &sessionManager, fileId] {
    //     auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
    //     if (currentTorrentIdOpt.has_value()) {
    //         sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId,
    //         lt::default_priority);
    //     }
    // });
    // priorityMenu->addAction(defaultPriority);

    // QAction* lowPriority = new QAction(Priorities::LOW, this);
    // connect(lowPriority, &QAction::triggered, this, [this, &sessionManager, fileId] {
    //     auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
    //     if (currentTorrentIdOpt.has_value()) {
    //         sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId, lt::low_priority);
    //     }
    // });
    // priorityMenu->addAction(lowPriority);

    // QAction* topPriority = new QAction(Priorities::HIGH, this);
    // connect(topPriority, &QAction::triggered, this, [this, &sessionManager, fileId] {
    //     auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
    //     if (currentTorrentIdOpt.has_value()) {
    //         sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId, lt::top_priority);
    //     }
    // });
    // priorityMenu->addAction(topPriority);

    // mainMenu.exec(ui->fileView->viewport()->mapToGlobal(pos));
}

void FileListWidget::setupTableView()
{
    ui->treeView->setModel(&m_fileModel);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(ui->treeView->header()->count() - 1,
                                                 QHeaderView::Stretch);
    ui->treeView->setAnimated(true);

    ui->treeView->setItemDelegateForColumn(static_cast<int>(FileFields::STATUS), &m_statusDelegate);
    ui->treeView->setItemDelegateForColumn(static_cast<int>(FileFields::PROGRESS), &m_itemDelegate);
    ui->treeView->setItemDelegateForColumn(static_cast<int>(FileFields::PRIORITY),
                                           &m_priorityDelegate);
}
