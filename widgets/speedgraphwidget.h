#ifndef SPEEDGRAPHWIDGET_H
#define SPEEDGRAPHWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QLineSeries>

class QChartView;
class QLineSeries;

namespace Ui
{
class SpeedGraphWidget;
}

class SpeedGraphWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit SpeedGraphWidget(QWidget *parent = nullptr);
    ~SpeedGraphWidget();

    enum Tab
    {
        ONE_MINUTE,
        FIVE_MINUTES,
        FIFTEEN_MINUTES,
        ONE_HOUR,
        TWELVE_HOURS,
        ONE_DAY
    };
  private slots:
    void on_periodComboBox_activated(int index);

  private:
    Ui::SpeedGraphWidget *ui;

    Tab m_currentTab{ONE_MINUTE};
    Tab m_prevTab{FIVE_MINUTES};

    QPointer<QChartView> m_chartView;

    QLineSeries *m_downloadSeries{nullptr};
    QLineSeries *m_uploadSeries{nullptr};

    QList<QPointF> m_downloadList;
    QList<QPointF> m_uploadList;

    void addLine(int download, int upload); // should pass in mb/s i think
    void addLineTest(int download, int upload);
};

#endif // SPEEDGRAPHWIDGET_H
