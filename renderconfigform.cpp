#include "renderconfigform.h"
#include "ui_renderconfigform.h"

using namespace RenderConf;

RenderConfigForm::RenderConfigForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RenderConfigForm)
{
    ui->setupUi(this);
}

RenderConfigForm::~RenderConfigForm()
{
    delete ui;
}

RenderConf::RenderConfig RenderConfigForm::GetConfig()
{

    RenderConfig Rend;

    Rend.NumSteps = ui->spbnSteps->value();
    Rend.Sampler = ui->cbSampler->currentIndex();
    Rend.Vae = ui->rbFullVae->isChecked() ? VaeSel::Normal : VaeSel::Tiny;

    return Rend;


}
