#include "gui/widgets/filelistwidget.h"
#include "core/utils/priority.h"
#include "ui_filelistwidget.h"
#include "core/controllers/sessionmanager.h"
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include "core/utils/settingsvalues.h"
#include <QSettings>

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
    setupHeader();
}

FileListWidget::~FileListWidget() { delete ui; }

void FileListWidget::setFiles(const QList<File> &files) { m_fileModel.setFiles(files); }

void FileListWidget::clearFiles() { m_fileModel.clearFiles(); }

void FileListWidget::contextMenuRequested(const QPoint &pos)
{
    auto index = ui->treeView->indexAt(pos);
    if (!index.isValid())
    {
        qDebug() << "Invalid index in " << __FUNCTION__;
        return;
    }

    // auto fileId = m_fileModel.getFileId(index.row());
    auto     fileId         = index.data(Qt::UserRole + 1).toInt();
    auto    &sessionManager = SessionManager::instance();
    QMenu    mainMenu(this);
    QAction *renameAction = new QAction(tr("Rename"), this);
    connect(renameAction, &QAction::triggered, this,
            [this, &sessionManager, fileId]
            {
                bool    ok{};
                QString text = QInputDialog::getText(this, tr("Renaming"), tr("New file name:"),
                                                     QLineEdit::Normal, "", &ok);
                if (ok && !text.isEmpty())
                {
                    auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
                    if (currentTorrentIdOpt.has_value())
                    {
                        sessionManager.renameFile(*currentTorrentIdOpt, fileId, text);
                    }
                }
            });
    mainMenu.addAction(renameAction);

    QMenu   *priorityMenu         = mainMenu.addMenu(tr("Priority"));
    QAction *dontDownloadPriority = new QAction(Priorities::DONT_DOWNLOAD, this);
    connect(dontDownloadPriority, &QAction::triggered, this,
            [this, &sessionManager, fileId]
            {
                auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
                if (currentTorrentIdOpt.has_value())
                {
                    sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId,
                                                      lt::dont_download);
                }
            });
    priorityMenu->addAction(dontDownloadPriority);

    QAction *defaultPriority = new QAction(Priorities::DEFAULT, this);
    connect(defaultPriority, &QAction::triggered, this,
            [this, &sessionManager, fileId]
            {
                auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
                if (currentTorrentIdOpt.has_value())
                {
                    sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId,
                                                      lt::default_priority);
                }
            });
    priorityMenu->addAction(defaultPriority);

    QAction *lowPriority = new QAction(Priorities::LOW, this);
    connect(lowPriority, &QAction::triggered, this,
            [this, &sessionManager, fileId]
            {
                auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
                if (currentTorrentIdOpt.has_value())
                {
                    sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId,
                                                      lt::low_priority);
                }
            });
    priorityMenu->addAction(lowPriority);

    QAction *topPriority = new QAction(Priorities::HIGH, this);
    connect(topPriority, &QAction::triggered, this,
            [this, &sessionManager, fileId]
            {
                auto currentTorrentIdOpt = sessionManager.getCurrentTorrentId();
                if (currentTorrentIdOpt.has_value())
                {
                    sessionManager.changeFilePriority(*currentTorrentIdOpt, fileId,
                                                      lt::top_priority);
                }
            });
    priorityMenu->addAction(topPriority);

    mainMenu.exec(ui->treeView->viewport()->mapToGlobal(pos));
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

void FileListWidget::setupHeader()
{
    QSettings settings;
    QByteArray headerState = settings.value(SettingsNames::DATA_FILES_HEADER).toByteArray();
    QHeaderView* header = ui->treeView->header();
    if (!headerState.isEmpty()) {
        header->restoreState(headerState);
    }
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header, &QHeaderView::customContextMenuRequested, this, &FileListWidget::headerMenuRequested);
}

void FileListWidget::headerMenuRequested(const QPoint &pos)
{
    auto* table = ui->treeView;
    auto* header = table->header();
    bool isChanged = false;

    QMenu menu(this);
    for (auto i = 0; i < header->count(); ++i) {
        QString headerName = table->model()->headerData(i, Qt::Horizontal).toString();
        bool isHidden = header->isSectionHidden(i);

        QAction* action = menu.addAction(headerName);
        action->setCheckable(true);
        action->setChecked(!isHidden);
        connect(action, &QAction::triggered, this, [header, i, isHidden, &isChanged]() mutable {
            header->setSectionHidden(i, !isHidden);
            isChanged = true;
        });
    }
    menu.exec(table->mapToGlobal(pos));

    if (isChanged) {
        QSettings settings;
        QByteArray headerState = header->saveState();
        settings.setValue(SettingsNames::DATA_FILES_HEADER, headerState);
    }
}
