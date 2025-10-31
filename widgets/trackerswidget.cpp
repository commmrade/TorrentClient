#include "trackerswidget.h"
#include "ui_trackerswidget.h"
#include <QSettings>
#include "settingsvalues.h"
#include <QMenu>

TrackersWidget::TrackersWidget(QWidget *parent) : QWidget(parent), ui(new Ui::TrackersWidget)
{
    ui->setupUi(this);

    ui->trackersView->setModel(&m_trackerModel);
    ui->trackersView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->trackersView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->trackersView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    setupHeader();
}

TrackersWidget::~TrackersWidget() { delete ui; }

void TrackersWidget::setTrackers(const QList<Tracker> &trackers)
{
    m_trackerModel.setTrackers(trackers);
}

void TrackersWidget::clearTrackers() { m_trackerModel.clearTrackers(); }

void TrackersWidget::setupHeader()
{
    QSettings settings;
    QByteArray headerState = settings.value(SettingsNames::DATA_TRACKERS_HEADER).toByteArray();
    QHeaderView* header = ui->trackersView->horizontalHeader();
    if (!headerState.isEmpty()) {
        header->restoreState(headerState);
    }
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header, &QHeaderView::customContextMenuRequested, this, &TrackersWidget::headerMenuRequested);
}

void TrackersWidget::headerMenuRequested(const QPoint &pos)
{
    auto* table = ui->trackersView;
    auto* header = table->horizontalHeader();
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
        settings.setValue(SettingsNames::DATA_TRACKERS_HEADER, headerState);
    }
}
