#include "deletetorrentdialog.h"
#include "ui_deletetorrentdialog.h"

DeleteTorrentDialog::DeleteTorrentDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::DeleteTorrentDialog)
{
    ui->setupUi(this);
}

DeleteTorrentDialog::~DeleteTorrentDialog() { delete ui; }

bool DeleteTorrentDialog::getRemoveWithContens() const { return ui->removeBox->isChecked(); }
