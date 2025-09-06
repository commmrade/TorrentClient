#ifndef SPEEDGRAPHWIDGET_H
#define SPEEDGRAPHWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QLineSeries>

class QChartView;
class QLineSeries;

namespace Ui {
class SpeedGraphWidget;
}

class SpeedGraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpeedGraphWidget(QWidget *parent = nullptr);
    ~SpeedGraphWidget();
private:
    Ui::SpeedGraphWidget *ui;

    QPointer<QChartView> m_chartView;

    QLineSeries* m_downloadSeries{nullptr};
    QLineSeries* m_uploadSeries{nullptr};

    void addLine(int download, int upload); // should pass in mb/s i think
};

#endif // SPEEDGRAPHWIDGET_H
