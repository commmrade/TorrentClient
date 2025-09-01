#include "trackerswidget.h"
#include "ui_trackerswidget.h"

TrackersWidget::TrackersWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TrackersWidget)
{
    ui->setupUi(this);

    ui->trackersView->setModel(&m_trackerModel);
    ui->trackersView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->trackersView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->trackersView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

TrackersWidget::~TrackersWidget()
{
    delete ui;
}

void TrackersWidget::setTrackers(const QList<Tracker> &trackers)
{
    m_trackerModel.setTrackers(trackers);
}

void TrackersWidget::clearTrackers()
{
    m_trackerModel.clearTrackers();
}
