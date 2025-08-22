#include "generalinfowidget.h"
#include "ui_generalinfowidget.h"
#include "utils.h"
#include <QDateTime>

GeneralInfoWidget::GeneralInfoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GeneralInfoWidget)
{
    ui->setupUi(this);

    ui->timeActValue->setText("-");
    ui->downloadedValue->setText("-");
    ui->downSpeedValue->setText("-");
    ui->downloadLimValue->setText("-");
    ui->etaValue->setText("-");
    ui->uploadedValue->setText("-");
    ui->upSpeedValue->setText("-");
    ui->uploadLimValue->setText("-");
    ui->connectionsValue->setText("-");
    ui->seedsValue->setText("-");
    ui->peersValue->setText("-");

    ui->torSizeValue->setText("-");
    ui->startTimeValue->setText("-");
    ui->infoHashValue->setText("-");
    ui->savePathValue->setText("-");
    ui->piecesValue->setText("-");
    ui->completionValue->setText("-");
}

GeneralInfoWidget::~GeneralInfoWidget()
{
    delete ui;
}

void GeneralInfoWidget::setGeneralInfo(const TorrentInfo &tInfo, const InternetInfo &iInfo)
{
    {
        auto hrs = iInfo.activeTime / 3600;
        auto mins = iInfo.activeTime % 3600 / 60;
        auto secs = iInfo.activeTime % 60;
        QString timeStr;
        if (iInfo.activeTime == -1) {
            timeStr = "infinity";
        } else {
            timeStr = QString("%1:%2:%3").arg(hrs).arg(mins, 2, 10, '0').arg(secs, 2, 10, '0');
        }
        ui->timeActValue->setText(timeStr);
    }

    auto downloaded = bytesToHigher(iInfo.downloaded);
    ui->downloadedValue->setText(downloaded);

    auto downSpeed = bytesToHigherPerSec(iInfo.downSpeed);
    ui->downSpeedValue->setText(downSpeed);

    auto downLim = bytesToHigherPerSec(iInfo.downLimit);
    ui->downloadLimValue->setText(downLim);

    {
        auto etaSecs = iInfo.eta;
        auto hrs = etaSecs / 3600;
        auto mins = etaSecs % 3600 / 60;
        auto secs = etaSecs % 60;
        QString etaStr;
        if (etaSecs == -1) {
            etaStr = "infinity";
        } else {
            etaStr = QString("%1:%2:%3").arg(hrs).arg(mins).arg(secs);
        }
        ui->etaValue->setText(etaStr);
    }

    auto uploaded = bytesToHigher(iInfo.uploaded);
    ui->uploadedValue->setText(uploaded);

    auto upSpeed = bytesToHigherPerSec(iInfo.upSpeed);
    ui->upSpeedValue->setText(upSpeed);

    auto upLim = bytesToHigherPerSec(iInfo.upLimit);
    ui->uploadLimValue->setText(upLim);

    auto conns = QString::number(iInfo.connections);
    ui->connectionsValue->setText(conns);

    auto seeds = QString::number(iInfo.seeds);
    ui->seedsValue->setText(seeds);

    auto peers = QString::number(iInfo.peers);
    ui->peersValue->setText(peers);

    //

    auto torSize = bytesToHigher(tInfo.size);
    ui->torSizeValue->setText(torSize);


    QDateTime timestamp;
    timestamp.setSecsSinceEpoch(tInfo.startTime);
    ui->startTimeValue->setText(timestamp.toString("dd.MM.yyyy hh:mm"));

    auto infoHash = tInfo.hashBest;
    ui->infoHashValue->setText(infoHash);

    auto savePath = tInfo.savePath;
    ui->savePathValue->setText(savePath);

    auto comm = tInfo.comment;
    ui->commentValue->setText(comm);

    auto pieces = QString::number(tInfo.piecesCount);
    auto piecesStr = pieces + " x " + bytesToHigher(tInfo.pieceSize);
    ui->piecesValue->setText(piecesStr);

    if (tInfo.completedTime.has_value()) {
        QDateTime timestamp;
        timestamp.setSecsSinceEpoch(tInfo.completedTime.value());
        ui->completionValue->setText(timestamp.toString("dd.MM.yyyy hh:mm"));
    }
}

void GeneralInfoWidget::clearGeneralInfo()
{
    ui->timeActValue->setText("-");
    ui->downloadedValue->setText("-");
    ui->downSpeedValue->setText("-");
    ui->downloadLimValue->setText("-");
    ui->etaValue->setText("-");
    ui->uploadedValue->setText("-");
    ui->upSpeedValue->setText("-");
    ui->uploadLimValue->setText("-");
    ui->connectionsValue->setText("-");
    ui->seedsValue->setText("-");
    ui->peersValue->setText("-");

    ui->torSizeValue->setText("-");
    ui->startTimeValue->setText("-");
    ui->infoHashValue->setText("-");
    ui->savePathValue->setText("-");
    ui->piecesValue->setText("-");
    ui->completionValue->setText("-");
}
