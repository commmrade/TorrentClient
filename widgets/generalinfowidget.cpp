#include "generalinfowidget.h"
#include "ui_generalinfowidget.h"

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
    // TODO: Convert to B mb kb gb depending on size
    auto timeActive = QString::number(iInfo.activeTime) + " s";
    ui->timeActValue->setText(timeActive);

    auto downloaded = QString::number(iInfo.downloaded) + " B";
    ui->downloadedValue->setText(downloaded);

    auto downSpeed = QString::number(iInfo.downSpeed) + " B/s";
    ui->downSpeedValue->setText(downSpeed);

    auto downLim = QString::number(iInfo.downLimit) + " B";
    ui->downloadLimValue->setText(downLim);

    auto eta = QString::number(iInfo.eta) + " s";
    ui->etaValue->setText(eta);

    auto uploaded = QString::number(iInfo.uploaded);
    ui->uploadedValue->setText(uploaded);

    auto upSpeed = QString::number(iInfo.upSpeed);
    ui->upSpeedValue->setText(upSpeed);

    auto upLim = QString::number(iInfo.upLimit);
    ui->uploadLimValue->setText(upLim);

    auto conns = QString::number(iInfo.connections);
    ui->connectionsValue->setText(conns);

    auto seeds = QString::number(iInfo.seeds);
    ui->seedsValue->setText(seeds);

    auto peers = QString::number(iInfo.peers);
    ui->peersValue->setText(peers);

    //

    auto torSize = QString::number(tInfo.size) + " B";
    ui->torSizeValue->setText(torSize);

    auto startTime = QString::number(tInfo.startTime);
    ui->startTimeValue->setText(startTime);

    auto infoHash = tInfo.hashBest;
    ui->infoHashValue->setText(infoHash);

    auto savePath = tInfo.savePath;
    ui->savePathValue->setText(savePath);

    // auto comm = tInfo.comment; TODO: Add
    auto pieces = QString::number(tInfo.piecesCount);
    auto piecesSize = QString::number(tInfo.pieceSize);
    auto piecesStr = pieces + " x " + piecesSize + " B";
    ui->piecesValue->setText(piecesStr);

    auto complTime = "TODO"; // TODO: ADD FIELD
    ui->completionValue->setText(complTime);
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
