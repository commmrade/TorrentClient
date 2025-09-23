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
    m_downloadSeries->setName(tr("Download speed"));
    m_uploadSeries = new QLineSeries{};
    m_uploadSeries->setName(tr("Upload speed"));


    m_chartView->chart()->addSeries(m_downloadSeries); // takes ownership
    m_chartView->chart()->addSeries(m_uploadSeries); // takes ownership
    m_chartView->chart()->createDefaultAxes();
    m_chartView->chart()->axes(Qt::Horizontal).first()->setLabelsVisible(false);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    ui->verticalLayout->addWidget(m_chartView);

    auto* verticalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Vertical).first());
    verticalAxis->setLabelFormat("%.0f B/s");
    // auto* horizontalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Horizontal).first());

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

// static void scaleSeries(QLineSeries* series, double factor) {
//     auto points = series->points();
//     for (auto& p : points) {
//         p.setY(p.y() / factor);
//     }
//     series->replace(points);
// }

static void scaleSeries(QLineSeries* series, double factor) {
    auto points = series->points();
    for (auto& p : points) {
        p.setY(p.y() * factor);
    }
    series->replace(points);
}


void SpeedGraphWidget::addLine(int download, int upload) {
    qreal lastX = m_downloadList.size();
    m_downloadList.push_back({QPointF{lastX, (qreal)download}});
    m_uploadList.push_back(QPointF{lastX, (qreal)upload});


    auto* verticalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Vertical).first());
    auto* horizontalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Horizontal).first());

    int step;
    switch (m_currentTab) {
        case ONE_MINUTE: step = 1; break;
        case FIVE_MINUTES: step = 5; break;
        case FIFTEEN_MINUTES: step = 15; break;
        case ONE_HOUR: step = 60; break;
        case TWELVE_HOURS: step = 720; break;
        case ONE_DAY: step = 1440; break;
        default: step = 1; break;
    }


    // Size is 'step' times smaller for different views
    // If size of lsit is 1024 points, it is 1024 / 15 points for 15 min timeframe
    int seriesSize = m_downloadList.size() / step;

    // Graph is 60 points wide
    // THese should be used as firstRangePos * step in first branch, because it is scaled, to get the actual index you multiply (1024/15) * step
    int firstRangePos = seriesSize > 60 ? seriesSize - 60 : 0;
    int lastRangePos = seriesSize > 60 ? seriesSize : 60;


    if (m_prevTab != m_currentTab) {
        // TIme format is changed, reset series
        m_downloadSeries->clear();
        m_uploadSeries->clear();

        // Lambda for figuring out max value in a list with steps as 'step'
        auto maxInList = [firstRangePos, step, this](const QList<QPointF> series) -> double {
            double max = 0.0;
            for (auto i = firstRangePos * step; i < series.size(); i += step) {
                max = std::max(max, series[i].y());
            }
            return max;
        };

        QString targetFormat = "%.0f B/s";
        auto maxSpeedInRangeBytes = std::max(maxInList(m_downloadList), maxInList(m_uploadList));

        double targetScale = 1.0;
        if (maxSpeedInRangeBytes >= utils::DBYTES_IN_MB) {
            targetFormat = "%.1f MB/s";
            targetScale = utils::DBYTES_IN_MB;
        } else if (maxSpeedInRangeBytes >= utils::DBYTES_IN_KB) {
            targetFormat = "%.1f KB/s";
            targetScale = utils::DBYTES_IN_KB;
        }

        // No point in optimizing really, since it is a 1 time operation
        verticalAxis->setMax((maxSpeedInRangeBytes / targetScale) * 1.1);
        verticalAxis->setLabelFormat(targetFormat);

        // Append scaled value from list to series
        for (auto i = 0; i < m_downloadList.size(); i += step) {
            auto downloadPoint = m_downloadList[i];
            auto uploadPoint = m_uploadList[i];
            m_downloadSeries->append(downloadPoint.x() / step, downloadPoint.y() / targetScale);
            m_uploadSeries->append(uploadPoint.x() / step, uploadPoint.y() / targetScale);
        }

        m_prevTab = m_currentTab;
    } else {
        auto maxInSeries = [firstRangePos, this](const QLineSeries* series) -> double {
            double max = 0.0;

            auto points = series->points();
            auto size = points.size();
            // NOTICE: No stepping here, since at this point, we are using actual indices, since for example, for 15 min tf downloadSeries size is 1024/15 at this point
            for (auto i = firstRangePos; i < size; ++i) {
                max = std::max(max, points[i].y());
            }
            // FIXME: Use max element
            return max;
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

        if (m_downloadList.size() % step == 0) {
            m_downloadSeries->append(lastX / step, newDownloadY);
            m_uploadSeries->append(lastX / step, newUploadY);
        }

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
    }
    horizontalAxis->setRange(firstRangePos, lastRangePos);
}

void SpeedGraphWidget::on_periodComboBox_activated(int index)
{
    m_prevTab = std::exchange(m_currentTab, static_cast<Tab>(index));
}

