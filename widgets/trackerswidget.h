#ifndef TRACKERSWIDGET_H
#define TRACKERSWIDGET_H

#include <QWidget>
#include "trackertablemodel.h"

namespace Ui
{
class TrackersWidget;
}

struct Tracker;

class TrackersWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit TrackersWidget(QWidget *parent = nullptr);
    ~TrackersWidget();

    void setTrackers(const QList<Tracker> &trackers);
    void clearTrackers();

  private:
    Ui::TrackersWidget *ui;
    TrackerTableModel   m_trackerModel;

    void setupHeader();
    void headerMenuRequested(const QPoint &pos);
};

#endif // TRACKERSWIDGET_H
