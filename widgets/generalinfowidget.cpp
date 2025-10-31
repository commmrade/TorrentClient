#include "generalinfowidget.h"
#include "ui_generalinfowidget.h"
#include "utils.h"
#include <QDateTime>

GeneralInfoWidget::GeneralInfoWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::GeneralInfoWidget)
{
    ui->setupUi(this);

    clearGeneralInfo();
}

GeneralInfoWidget::~GeneralInfoWidget() { delete ui; }

void GeneralInfoWidget::setGeneralInfo(const TorrentInfo &tInfo, const InternetInfo &iInfo)
{
    {
        // auto    hrs  = iInfo.activeTime / 3600;
        // auto    mins = iInfo.activeTime % 3600 / 60;
        // auto    secs = iInfo.activeTime % 60;
        // QString timeStr;
        // if (iInfo.activeTime == -1)
        // {
        //     timeStr = tr("infinity");
        // }
        // else
        // {
        //     timeStr = QString("%1:%2:%3").arg(hrs).arg(mins, 2, 10, '0').arg(secs, 2, 10, '0');
        auto timeStr = utils::secsToFormattedTime(iInfo.activeTime);
        ui->timeActValue->setText(timeStr);
    }

    auto downloaded = utils::bytesToHigher(iInfo.downloaded);
    ui->downloadedValue->setText(downloaded);

    auto downSpeed = utils::bytesToHigherPerSec(iInfo.downSpeed);
    ui->downSpeedValue->setText(downSpeed);

    ui->downloadLimValue->setText(
        iInfo.downLimit == -1 ? tr("Unlimited") : utils::bytesToHigherPerSec(iInfo.downLimit));

    {
        auto etaStr = utils::secsToFormattedTime(iInfo.eta);
        ui->etaValue->setText(etaStr);
    }

    auto uploaded = utils::bytesToHigher(iInfo.uploaded);
    ui->uploadedValue->setText(uploaded);

    auto upSpeed = utils::bytesToHigherPerSec(iInfo.upSpeed);
    ui->upSpeedValue->setText(upSpeed);

    ui->uploadLimValue->setText(iInfo.upLimit == -1 ? tr("Unlimited")
                                                    : utils::bytesToHigherPerSec(iInfo.upLimit));

    auto ratio = QString::number(iInfo.ratio);
    ui->ratioValue->setText(ratio);

    auto conns = QString::number(iInfo.connections);
    ui->connectionsValue->setText(conns);

    auto seeds = QString::number(iInfo.seeds);
    ui->seedsValue->setText(seeds);

    auto peers = QString::number(iInfo.peers);
    ui->peersValue->setText(peers);

    //

    auto torSize = utils::bytesToHigher(tInfo.size);
    ui->torSizeValue->setText(torSize);

    QDateTime timestamp;
    timestamp.setSecsSinceEpoch(tInfo.startTime);
    ui->startTimeValue->setText(timestamp.toString("dd.MM.yyyy hh:mm"));

    auto infoHashV1 = tInfo.hashV1;
    ui->infoHashV1Value->setText(infoHashV1);

    auto infoHashV2 = tInfo.hashV2;
    ui->infoHashV2Value->setText(infoHashV2.isEmpty() ? "N/A" : infoHashV2);

    auto savePath = tInfo.savePath;
    ui->savePathValue->setText(savePath);

    auto comm = tInfo.comment;
    ui->commentValue->setText(comm);

    auto pieces    = QString::number(tInfo.piecesCount);
    auto piecesStr = pieces + " x " + utils::bytesToHigher(tInfo.pieceSize);
    ui->piecesValue->setText(piecesStr);

    if (tInfo.completedTime != 0)
    {
        QDateTime timestamp;
        timestamp.setSecsSinceEpoch(tInfo.completedTime);
        ui->completionValue->setText(timestamp.toString("dd.MM.yyyy hh:mm"));
    }
    else
    {
        ui->completionValue->setText(tr("Not finished"));
    }

    ui->piecesBar->update();
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
    ui->infoHashV1Value->setText("-");
    ui->savePathValue->setText("-");
    ui->piecesValue->setText("-");
    ui->completionValue->setText("-");

    ui->piecesBar->clearPieces();
}

void GeneralInfoWidget::setPieces(const lt::typed_bitfield<libtorrent::piece_index_t> &pieces,
                                  const std::vector<int> &downloadingPiecesIndices)
{
    ui->piecesBar->setPieces(pieces, downloadingPiecesIndices);
}
