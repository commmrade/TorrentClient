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
    m_downloadSeries->setName(tr("Download speed B/s"));
    m_uploadSeries = new QLineSeries{};
    m_uploadSeries->setName(tr("Upload speed B/s"));



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


static double toBytesfromFormat(double value, const QString& format) {
    if (format.contains("KB/s")) {
        return value * utils::DBYTES_IN_KB;
    } else if (format.contains("MB/s")) {
        return value * utils::DBYTES_IN_MB;
    }
    return value;
}
static double toValueByFormat(double bytes, const QString& format) {
    if (format.contains("KB/s")) {
        return bytes / utils::DBYTES_IN_KB;
    } else if (format.contains("MB/s")) {
        return bytes / utils::DBYTES_IN_MB;
    }
    return bytes;
}

static void scaleSeries(QLineSeries* series, double factor) {
    auto points = series->points();
    for (auto& p : points) {
        p.setY(p.y() * factor);
    }
    series->replace(points);
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

    auto maxInSeries = [firstRangePos](const QLineSeries* series) -> double {
        const auto& points = series->points();
        if (static_cast<int>(points.size()) <= firstRangePos) return 0.0;
        auto startIt = points.begin() + firstRangePos;
        auto maxIt = std::max_element(startIt, points.end(), [](const QPointF& a, const QPointF& b) { return a.y() < b.y(); });
        return maxIt->y(); // Can't throw, since we have an if statement guard
    };

    QString currentFormat = verticalAxis->labelFormat();

    double maxSpeedInRangeBytes = std::max(toBytesfromFormat(maxInSeries(m_downloadSeries), currentFormat), toBytesfromFormat(maxInSeries(m_uploadSeries), currentFormat));
    double maxSpeed = verticalAxis->max();
    double maxSpeedBytes = toBytesfromFormat(maxSpeed, currentFormat);


    // Figure out the target format depending on maxSpeed in range bytes
    QString targetFormat = "%.0f B/s";

    // Figure out current scale
    double currentScale = 1.0;
    if (currentFormat.contains("KB/s")) {
        currentScale = utils::DBYTES_IN_KB;
    } else if (currentFormat.contains("MB/s")) {
        currentScale = utils::DBYTES_IN_MB;
    }


    // Add new points scaled by the current scale, this way it is ok if format is changed later
    qreal newDownloadY = download / currentScale;
    qreal newUploadY = upload / currentScale;
    m_downloadSeries->append(lastX, newDownloadY);
    m_uploadSeries->append(lastX, newUploadY);

    // scale all series only if difference is big enough, why waste resources?
    double targetScale = 1.0;
    if (maxSpeedInRangeBytes >= utils::DBYTES_IN_MB) {
        targetFormat = "%.1f MB/s";
        targetScale = utils::DBYTES_IN_MB;
    } else if (maxSpeedInRangeBytes >= utils::DBYTES_IN_KB) {
        targetFormat = "%.1f KB/s";
        targetScale = utils::DBYTES_IN_KB;
    }
    double factor = currentScale / targetScale;

    if (currentFormat != targetFormat) { // expensive operation, reduce uage
        scaleSeries(m_downloadSeries, factor);
        scaleSeries(m_uploadSeries, factor);
        verticalAxis->setLabelFormat(targetFormat);
    }

    if (std::abs(maxSpeedBytes - maxSpeedInRangeBytes * 1.1) > std::numeric_limits<double>::epsilon()) {
        verticalAxis->setMax((maxSpeedInRangeBytes / targetScale) * 1.1); // i think this is an expensive operation so should do as few of these as possible
    }

    horizontalAxis->setRange(firstRangePos, lastRangePos);
}
// Optimizations gave about 18% of performance boost
