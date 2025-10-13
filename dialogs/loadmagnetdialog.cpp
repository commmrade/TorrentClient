#include "loadmagnetdialog.h"
#include "ui_loadmagnetdialog.h"

LoadMagnetDialog::LoadMagnetDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoadMagnetDialog)
{
    ui->setupUi(this);
}

LoadMagnetDialog::~LoadMagnetDialog() { delete ui; }

QList<QString> LoadMagnetDialog::getMagnets() const
{
    QString magnetsStr = ui->textEdit->toPlainText();
    QStringList magnets = magnetsStr.split('\n');
    return magnets;
}
