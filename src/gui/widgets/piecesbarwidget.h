#ifndef PIECESBARWIDGET_H
#define PIECESBARWIDGET_H

#include <QWidget>
#include <libtorrent/libtorrent.hpp>
namespace Ui
{
class PiecesBarWidget;
}

class PiecesBarWidget final : public QWidget
{
    Q_OBJECT

    static constexpr inline int BAR_WIDTH_PX = 900;

  public:
    explicit PiecesBarWidget(QWidget *parent = nullptr);
    ~PiecesBarWidget();

    void setPieces(const lt::typed_bitfield<lt::piece_index_t> &pieces,
                   const std::vector<int>                      &downloadingPiecesIndices);
    void clearPieces();

    void paintEvent(QPaintEvent *event) override;

  private:
    Ui::PiecesBarWidget *ui;

    lt::typed_bitfield<lt::piece_index_t> m_pieces;
    std::vector<int>                      m_downloadingPiecesIndices;
};

#endif // PIECESBARWIDGET_H
