#include "speedgraphwidget.h"
#include "ui_speedgraphwidget.h"
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QTimer>
#include "sessionmanager.h"
#include "utils.h"
#include <execution>

SpeedGraphWidget::SpeedGraphWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SpeedGraphWidget)
{
    ui->setupUi(this);

    srand(time(NULL));

    m_chartView = new QChartView(this);
    m_downloadSeries = new QLineSeries{};
    m_downloadSeries->setName("Download speed B/s");
    m_uploadSeries = new QLineSeries{};
    m_uploadSeries->setName("Upload speed B/s");



    m_chartView->chart()->addSeries(m_downloadSeries); // takes ownership
    m_chartView->chart()->addSeries(m_uploadSeries); // takes ownership
    m_chartView->chart()->createDefaultAxes();
    m_chartView->chart()->axes(Qt::Horizontal).first()->setLabelsVisible(false);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    ui->verticalLayout->addWidget(m_chartView);

    auto* verticalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Vertical).first());
    verticalAxis->setLabelFormat("%.0f B/s");

    auto& sessionManager = SessionManager::instance(); // TODO: Maybe call this from torrentWidget to reduce coupling between this class and SessionManager
    connect(&sessionManager, &SessionManager::chartPoint, this, &SpeedGraphWidget::addLine);
}

SpeedGraphWidget::~SpeedGraphWidget()
{
    delete ui;
}


void SpeedGraphWidget::addLine(int download, int upload)
{
    auto* verticalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Vertical).first());
    auto* horizontalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Horizontal).first());

    // Next x pos of graph
    qreal lastX = m_downloadSeries->points().isEmpty() ? 0 : m_downloadSeries->points().constLast().x() + 1;

    int seriesSize = m_downloadSeries->points().size();
    // Graph is 60 points wide
    int firstRangePos = seriesSize > 60 ? seriesSize - 60 : 0;
    int lastRangePos = seriesSize > 60 ? seriesSize : 60;

    // We are converting into format of newly added point (TODO: Change to max)
    int maxSpeed = std::max(download, upload);
    QString targetFormat = "%.0f B/s";
    double targetScale = 1.0;
    if (maxSpeed >= 1024 * 1024) {
        targetFormat = "%.0f MB/s";
        targetScale = 1024.0 * 1024.0;
    } else if (maxSpeed >= 1024) {
        targetFormat = "%.0f KB/s";
        targetScale = 1024.0;
    }

    // Deducing current scale depending on the label format
    QString currentFormat = verticalAxis->labelFormat();
    double currentScale = 1.0;
    if (currentFormat.contains("KB/s")) {
        currentScale = 1024.0;
    } else if (currentFormat.contains("MB/s")) {
        currentScale = 1024.0 * 1024.0;
    }

    // Use this to scale series to target scale
    double factor = currentScale / targetScale;
    bool unitChanged = currentFormat != targetFormat; // Scale series only if something is changed

    auto scaleSeries = [factor](QLineSeries* series) {
        auto points = series->points();
        for (auto& p : points) {
            p.setY(p.y() * factor);
        }
        series->replace(points);
    };

    if (unitChanged) {
        scaleSeries(m_downloadSeries);
        scaleSeries(m_uploadSeries);
        // Update series and change label format
        verticalAxis->setLabelFormat(targetFormat);
    }

    // Append new points
    qreal newDownloadY = download / targetScale;
    qreal newUploadY = upload / targetScale;
    m_downloadSeries->append(lastX, newDownloadY);
    m_uploadSeries->append(lastX, newUploadY);

    auto maxInSeries = [firstRangePos](const QLineSeries* series) -> double {
        const auto& points = series->points();
        if (static_cast<int>(points.size()) <= firstRangePos) return 0.0;
        auto startIt = points.begin() + firstRangePos;
        auto maxIt = std::max_element(startIt, points.end(), [](const QPointF& a, const QPointF& b) { return a.y() < b.y(); });
        return maxIt->y(); // Can't throw, since we have an if statement guard
    };

    // New points are added to the series, no point in including them in max({})
    // Notice: std::max can accept init_list with as many values as u cant
    double maxY = std::max(maxInSeries(m_downloadSeries), maxInSeries(m_uploadSeries));
    verticalAxis->setMax(maxY * 1.1);

    horizontalAxis->setRange(firstRangePos, lastRangePos);
}
