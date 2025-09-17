#include "piecesbarwidget.h"
#include "ui_piecesbarwidget.h"
#include <QPainter>


PiecesBarWidget::PiecesBarWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PiecesBarWidget)
{
    ui->setupUi(this);
}

PiecesBarWidget::~PiecesBarWidget()
{
    delete ui;
}

void PiecesBarWidget::clearPieces()
{
    m_pieces.clear();
    m_downloadingPiecesIndices.clear();
    update();
}

void PiecesBarWidget::setPieces(const lt::typed_bitfield<libtorrent::piece_index_t> &pieces, const std::vector<int>& downloadingPiecesIndices)
{
    m_pieces = pieces;
    m_downloadingPiecesIndices = downloadingPiecesIndices;
    update();
}

void PiecesBarWidget::paintEvent(QPaintEvent *event)
{


    QPainter painter(this);

    // Dimensions of bar
    int const startX = width() / 10;
    int const endX = width() - (width() / 10);

    int const startY = height() / 3;
    int const endY = height() - (height() / 3);

    int const barHeight = endY - startY;

    if (m_pieces.empty()) { // Draw empty background in case when there are no pieces to not waste resources
        QRect background{startX, startY, BAR_WIDTH_PX, barHeight};
        painter.fillRect(background, QBrush{QColor::fromRgb(199, 199, 199)});
        return;
    }

    // int barWidthPx = BAR_WIDTH_PX;
    int barWidthPx = width() - startX;
    qDebug() << barWidthPx << width();
    int const piecesPerPixel = m_pieces.size() / barWidthPx; // How many pieces will be considered in 1 pixel

    QSet<int> hashedDownloadingPieces; // idx bool

    int startPos = startX;
    int endPos = startX + barWidthPx; // Dimensions of the bar on a widget

    constexpr QColor backgroundColor = QColor{199, 199, 199};
    constexpr QColor finishedColor = QColor{50, 50, 255};
    constexpr QColor downloadingColor = QColor{50, 255, 50};
    if (piecesPerPixel) { // if pieces per pixel >= 1
        int const chunks = m_pieces.size() / piecesPerPixel;

        for (int i = 0; i < chunks; ++i) {
            int finishedPieces = 0;
            bool isDownloading = false;

            // Iterate until where
            int pidxUntil = piecesPerPixel * i + piecesPerPixel;
            if (startPos >= endPos - 1) { // if its the last iteration
                pidxUntil = m_pieces.size(); // fix the end to the end of pieces, so we wont go over
            }

            for (auto pidx = piecesPerPixel * i; pidx < pidxUntil; ++pidx) {
                if (m_pieces[pidx]) {
                    ++finishedPieces;
                    continue;
                }
                // If piece is downloading set the flag, cache the result if its not
                if (hashedDownloadingPieces.contains(pidx)) {
                    isDownloading = true;
                } else {
                    auto iter = std::find_if(m_downloadingPiecesIndices.begin(), m_downloadingPiecesIndices.end(), [pidx](const auto value) {
                        return value == pidx;
                    });
                    if (iter != m_downloadingPiecesIndices.end()) {
                        hashedDownloadingPieces.insert(pidx);
                        isDownloading = true;
                    }
                }
            }

            QRect piece{startPos, startY, 1, barHeight};
            QColor color;/* isDownloaded ? QColor::fromRgb(50, 50, 255) : QColor::fromRgb(199, 199, 199);*/

            if (finishedPieces == 0 && !isDownloading) {
                // color = QColor::fromRgb(199, 199, 199);
                color = backgroundColor;
            } else if (finishedPieces == pidxUntil - piecesPerPixel * i) {
                // color = QColor::fromRgb(50, 50, 255);
                color = finishedColor;
            } else if (isDownloading || finishedPieces) {
                // color = QColor::fromRgb(50, 255, 50);
                color = downloadingColor;
            }
            painter.fillRect(piece, QBrush{color});
            ++startPos;
            // qDebug() << "finished Pieces" << finishedPieces << piecesPerPixel;
            if (pidxUntil == m_pieces.size()) { // finished
                break;
            }
        }
    } else {
        int const pixelsInPiece = barWidthPx / m_pieces.size();

        for (int i = 0; i < m_pieces.size(); ++i) {
            bool isDownloaded = false;
            bool isDownloading = false;

            // Edge case, we are getting close to the end
            if (startPos >= endPos - pixelsInPiece) {
                for (; i < m_pieces.size(); ++i) {
                    if (m_pieces[i]) {
                        isDownloaded = true;
                    }
                }

                if (hashedDownloadingPieces.contains(i)) {
                    isDownloading = true;
                } else {
                    auto iter = std::find_if(m_downloadingPiecesIndices.begin(), m_downloadingPiecesIndices.end(), [i](const auto value) {
                        return value == i;
                    });
                    if (iter != m_downloadingPiecesIndices.end()) {
                        hashedDownloadingPieces.insert(i);
                        isDownloading = true;
                    }
                }
                QColor color;

                // TODO: Убраь это дебильное дублирование кода
                if (!isDownloaded && !isDownloading) {
                    // color = QColor::fromRgb(199, 199, 199);
                    color = backgroundColor;
                } else if (isDownloaded && !isDownloading) {
                    // color = QColor::fromRgb(50, 50, 255);
                    color = finishedColor;
                } else if (!isDownloaded && isDownloading) {
                    // color = QColor::fromRgb(50, 255, 50);
                    color = downloadingColor;
                } // Cant be both isDownloaded and isDownloading (hope so)

                QRect rect{startPos, startY, pixelsInPiece, barHeight};
                painter.fillRect(rect, QBrush{color});
                break;
            }

            isDownloaded = m_pieces[i];
            if (hashedDownloadingPieces.contains(i)) {
                isDownloading = true;
            } else {
                auto iter = std::find_if(m_downloadingPiecesIndices.begin(), m_downloadingPiecesIndices.end(), [i](const auto value) {
                    return value == i;
                });
                if (iter != m_downloadingPiecesIndices.end()) {
                    hashedDownloadingPieces.insert(i);
                    isDownloading = true;
                }
            }


            QRect rect{startPos, startY, pixelsInPiece, barHeight};
            QColor color;
            if (!isDownloaded && !isDownloading) {
                // color = QColor::fromRgb(199, 199, 199);
                color = backgroundColor;
            } else if (isDownloaded && !isDownloading) {
                // color = QColor::fromRgb(50, 50, 255);
                color = finishedColor;
            } else if (!isDownloaded && isDownloading) {
                // color = QColor::fromRgb(50, 255, 50);
                color = downloadingColor;
            } // Cant be both isDownloaded and isDownloading (hope so)

            painter.fillRect(rect, QBrush{color});

            startPos += pixelsInPiece;
        }
    }
}
