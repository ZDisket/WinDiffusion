#include "newcanvasdialog.h"
#include "ui_newcanvasdialog.h"

NewCanvasDialog::NewCanvasDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewCanvasDialog)
{
    ui->setupUi(this);
}

NewCanvasDialog::~NewCanvasDialog()
{
    delete ui;
}

void NewCanvasDialog::on_buttonBox_accepted()
{
    settings.color = ui->rbFillSolidColor->isChecked() ? ui->colSolidFill->color() : Qt::transparent;
    settings.size = QSize(ui->spbWidth->value(), ui->spbHeight->value());


}


void NewCanvasDialog::on_NewCanvasDialog_accepted()
{
    on_buttonBox_accepted();
}

