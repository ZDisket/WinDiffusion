#include "layersettingsdialog.h"
#include "ui_layersettingsdialog.h"

// Yes, I'm a cnile.
#define BOOL_TO_CHECKED(in_b) in_b ? Qt::Checked : Qt::Unchecked


LayerSettingsDialog::LayerSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LayerSettingsDialog)
{
    ui->setupUi(this);
}

LayerSettingsDialog::~LayerSettingsDialog()
{
    delete ui;

}

void LayerSettingsDialog::Update()
{
    ui->ledtLayerName->setText(settings.name);
    ui->sliLayerOpacity->setValue(settings.opacity);
    ui->chkRenderable->setCheckState(BOOL_TO_CHECKED(settings.render));
    ui->chkVisible->setCheckState(BOOL_TO_CHECKED(settings.visible));

}

void LayerSettingsDialog::on_buttonBox_accepted()
{
    settings.name = ui->ledtLayerName->text();
    settings.opacity = ui->sliLayerOpacity->value();
    settings.render = ui->chkRenderable->isChecked();
    settings.visible = ui->chkVisible->isChecked();

}


void LayerSettingsDialog::on_sliLayerOpacity_valueChanged(int value)
{
    ui->lblOpacityPerc->setText(QString::number(value) + "%");
}

