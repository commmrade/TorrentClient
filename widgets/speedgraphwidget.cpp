#include "speedgraphwidget.h"
#include "ui_speedgraphwidget.h"
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QTimer>
#include "utils.h"
#include "sessionmanager.h"

SpeedGraphWidget::SpeedGraphWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SpeedGraphWidget)
{
    ui->setupUi(this);

    srand(time(NULL));

    m_chartView = new QChartView(this);
    m_downloadSeries = new QLineSeries{};
    m_downloadSeries->setName("Download speed KB/s");
    m_uploadSeries = new QLineSeries{};
    m_uploadSeries->setName("Upload speed KB/s");



    m_chartView->chart()->addSeries(m_downloadSeries); // takes ownership
    m_chartView->chart()->addSeries(m_uploadSeries); // takes ownership
    m_chartView->chart()->createDefaultAxes();
    m_chartView->chart()->axes(Qt::Horizontal).first()->setLabelsVisible(false);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    ui->verticalLayout->addWidget(m_chartView);

    auto* verticalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Vertical).first());
    verticalAxis->setLabelFormat("%.0f KB/s");

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
    // TODO: logic for scaling vertical axis for mb kb gb and etc

    double downloadKb = download / 1024.0; // to kb
    double uploadKb = upload / 1024.0; // to kb

    qreal lastX = 0;
    if (!m_downloadSeries->points().isEmpty()) {
        lastX = m_downloadSeries->points().constLast().x() + 1;
    }

    if (downloadKb > verticalAxis->max()) {
        verticalAxis->setMax(downloadKb + 1);
    } else if (uploadKb > verticalAxis->max()) {
        verticalAxis->setMax(uploadKb + 1);
    }

    m_downloadSeries->append(lastX, downloadKb);
    m_uploadSeries->append(lastX, uploadKb);

    int firstRangePos = m_downloadSeries->points().size() > 60 ? m_downloadSeries->points().size() - 60 : 0;
    int lastRangePos = m_downloadSeries->points().size() > 60 ? m_downloadSeries->points().size() : 60;

    static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Horizontal).first())->setRange(firstRangePos, lastRangePos);
}



// void SpeedGraphWidget::addLine(int download, int upload)
// {
//     auto* verticalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Vertical).first());
//     // TODO: logic for scaling vertical axis for mb kb gb and etc
//     // download = download / 1024; // to kb
//     // upload = upload / 1024; // to kb

//     // Выяснить в какой системе данные щас (kb, mb, gb, etc)
//     auto downloadStr = bytesToHigherPerSec(download); // coneverts 1 * 1024 B/s into 1 KB/s and etc
//     // auto uploadStr = bytesToHigherPerSec(upload);

//     qDebug() << downloadStr << m_downloadSeries->name();
//     if (m_downloadSeries->name().contains("B/s") && !m_downloadSeries->name().contains("MB/s") && !m_downloadSeries->name().contains("KB/s")) {
//         if (downloadStr.contains("B/s") && !downloadStr.contains("MB/s") && !downloadStr.contains("KB/s")) {
//             // all good
//         } else if (downloadStr.contains("KB/s")) {
//             // Set max to max / 1024
//             // change name
//             m_downloadSeries->setName("Download speed KB/s");
//             m_uploadSeries->setName("Upload speed KB/s");
//             verticalAxis->setMax(verticalAxis->max() / 1024.0 + 1); // B/s to KB/s

//             download /= 1024.0;
//             // upload /= 1024.0;
//         } else if (downloadStr.contains("MB/s")) {

//             // qDebug() << "here";
//             m_downloadSeries->setName("Download speed MB/s");
//             m_uploadSeries->setName("Upload speed MB/s");

//             verticalAxis->setMax(verticalAxis->max() / 1024.0 / 1024.0 + 1); // B/s to KB/s
//             download /= 1024.0 * 1024.0;
//             // upload /= 1024.0 * 1024.0;
//         }
//     } else if (m_downloadSeries->name().contains("KB/s")) {
//         if (downloadStr.contains("B/s") && !downloadStr.contains("MB/s") && !downloadStr.contains("KB/s")) {
//             m_downloadSeries->setName("Download speed B/s");
//             m_uploadSeries->setName("Upload speed B/s");
//             verticalAxis->setMax(verticalAxis->max() * 1024.0 + 1); // KB/s to B/s

//         } else if (downloadStr.contains("KB/s")) {
//             // all good
//             download /= 1024.0;
//             // upload /= 1024.0;
//         } else if (downloadStr.contains("MB/s")) {
//             m_downloadSeries->setName("Download speed MB/s");
//             m_uploadSeries->setName("Upload speed MB/s");

//             verticalAxis->setMax(verticalAxis->max() / 1024.0 + 1); // KB/s to MB/s
//             download /= 1024.0 * 1024.0;
//             // upload /= 1024.0 * 1024.0;
//         }
//     } else if (m_downloadSeries->name().contains("MB/s")) {
//         if (downloadStr.contains("B/s") && !downloadStr.contains("MB/s") && !downloadStr.contains("KB/s")) {
//             m_downloadSeries->setName("Download speed B/s");
//             m_uploadSeries->setName("Upload speed B/s");
//             verticalAxis->setMax(verticalAxis->max() * 1024.0 * 1024.0 + 1); // MB/s to B/s
//         } else if (downloadStr.contains("KB/s")) {
//             m_downloadSeries->setName("Download speed KB/s");
//             m_uploadSeries->setName("Upload speed KB/s");
//             download /= 1024.0;
//             // upload /= 1024.0;
//             verticalAxis->setMax(verticalAxis->max() * 1024.0 + 1); // MB/s to KB/s
//         } else if (downloadStr.contains("MB/s")) {
//             // all good
//             download /= 1024.0 * 1024.0;
//             // upload /= 1024.0 * 1024.0;
//         }
//     }

//     // TODO: Scale points in current view (60)

//     // qDebug() << downloadStr << uploadStr << m_downloadSeries->name() << m_uploadSeries->name();
//     // qDebug() << "=====";

//     qreal lastX = 0;
//     if (!m_downloadSeries->points().isEmpty()) {
//         lastX = m_downloadSeries->points().constLast().x() + 1;
//     }


//     if (download > verticalAxis->max()) {
//         verticalAxis->setMax(download + 1);
//     } else if (upload > verticalAxis->max()) {
//         // verticalAxis->setMax(upload + 1);
//     }

//     m_downloadSeries->append(lastX, download);
//     // m_uploadSeries->append(lastX, upload);

//     int firstRangePos = m_downloadSeries->points().size() > 60 ? m_downloadSeries->points().size() - 60 : 0;
//     int lastRangePos = m_downloadSeries->points().size() > 60 ? m_downloadSeries->points().size() : 60;

//     static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Horizontal).first())->setRange(firstRangePos, lastRangePos);
// }


