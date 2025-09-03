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
    // m_pieces.clear_all();
    m_pieces.clear();
}

void PiecesBarWidget::setPieces(const lt::typed_bitfield<libtorrent::piece_index_t> &pieces)
{
    m_pieces = pieces;
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

    int const piecesPerPixel = m_pieces.size() / BAR_WIDTH_PX; // How many pieces will be considered in 1 pixel

    int startPos = startX;
    int endPos = startX + BAR_WIDTH_PX;
    if (piecesPerPixel) { // if pieces per pixel >= 1
        int const chunks = m_pieces.size() / piecesPerPixel;
        for (int i = 0; i < chunks; ++i) {
            bool isDownloaded = true; // TODO: Add currently downloaded pieces

            int pidxUntil = piecesPerPixel * i + piecesPerPixel;
            if (startPos >= endPos - 1) { // if its the last iteration
                pidxUntil = m_pieces.size();
            }

            for (auto pidx = piecesPerPixel * i; pidx < pidxUntil; ++pidx) {
                if (!m_pieces[pidx]) {
                    isDownloaded = false;
                }
            }

            QRect piece{startPos, startY, 1, barHeight};
            QColor color = isDownloaded ? QColor::fromRgb(50, 50, 255) : QColor::fromRgb(199, 199, 199);
            painter.fillRect(piece, QBrush{color});
            ++startPos;

            if (pidxUntil == m_pieces.size()) break;
        }
    } else {
        int const pixelsInPiece = BAR_WIDTH_PX / m_pieces.size();

        for (int i = 0; i < m_pieces.size(); ++i) {
            if (startPos >= endPos - pixelsInPiece) {
                bool isDownloaded = true;
                for (; i < m_pieces.size(); ++i) {
                    if (!m_pieces[i]) {
                        isDownloaded = false;
                    }
                }

                QRect rect{startPos, startY, pixelsInPiece, barHeight};
                QColor color = isDownloaded ? QColor::fromRgb(0, 0, 255) : QColor::fromRgb(0, 0, 0);
                painter.fillRect(rect, QBrush{color});
                break;
            }

            bool isDownloaded = m_pieces[i];
            QRect rect{startPos, startY, pixelsInPiece, barHeight};

            QColor color = isDownloaded ? QColor::fromRgb(0, 0, 255) : QColor::fromRgb(0, 0, 0);
            painter.fillRect(rect, QBrush{color});

            startPos += pixelsInPiece;
        }
    }
}
