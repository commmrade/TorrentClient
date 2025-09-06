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

    // for (auto i = 0; i < 100; ++i) {
    //     m_downloadSeries->append(i, rand() % 50);
    //     m_uploadSeries->append(i, rand() % 20);
    // }

    // TODO:
    // pass series from session
    // a buton to open widget in torrent widget
    // test if many series

    m_chartView->chart()->addSeries(m_downloadSeries); // takes ownership
    m_chartView->chart()->addSeries(m_uploadSeries); // takes ownership
    m_chartView->chart()->createDefaultAxes();
    m_chartView->chart()->axes(Qt::Horizontal).first()->setLabelsVisible(false);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    ui->verticalLayout->addWidget(m_chartView);

    // static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Vertical).first())->setMax(260); // test

    auto& sessionManager = SessionManager::instance(); // TODO: Maybe call this from torrentWidget to reduce coupling between this class and SessionManager
    connect(&sessionManager, &SessionManager::chartPoint, this, &SpeedGraphWidget::addLine);
}

SpeedGraphWidget::~SpeedGraphWidget()
{
    delete ui;
}

void SpeedGraphWidget::addLine(int download, int upload)
{
    // TODO: logic for scaling vertical axis for mb kb gb and etc
    download = download / 1024; // to kb
    upload = upload / 1024; // to kb



    qreal lastX = 0;
    if (!m_downloadSeries->points().isEmpty()) {
        lastX = m_downloadSeries->points().constLast().x() + 1;
    }

    auto* verticalAxis = static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Vertical).first());
    if (download > verticalAxis->max()) {
        verticalAxis->setMax(download + 5);
    } else if (upload > verticalAxis->max()) {
        verticalAxis->setMax(upload + 5);
    }

    m_downloadSeries->append(lastX, download);
    m_uploadSeries->append(lastX, upload);

    int firstRangePos = m_downloadSeries->points().size() > 60 ? m_downloadSeries->points().size() - 60 : 0;
    int lastRangePos = m_downloadSeries->points().size() > 60 ? m_downloadSeries->points().size() : 60;

    static_cast<QValueAxis*>(m_chartView->chart()->axes(Qt::Horizontal).first())->setRange(firstRangePos, lastRangePos);
}
