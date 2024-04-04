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

void RenderConfigForm::SetConfig(const RenderConf::RenderConfig &InConf)
{
    ui->spbnSteps->setValue(InConf.NumSteps);
    ui->cbSampler->setCurrentIndex(InConf.Sampler);


    ui->rbFullVae->setChecked(InConf.Vae == VaeSel::Normal);
    ui->rbTinyVae->setChecked(InConf.Vae == VaeSel::Tiny);



}


ZFILE_OOVR(RenderConf::RenderConfig, RendCof)
{

    right >> RendCof.NumSteps;
    right >> RendCof.Sampler;

    int32_t vae;

    right >> vae;

    RendCof.Vae = (RenderConf::VaeSel)vae;

    return right;

}

ZFILE_IOVR(RenderConf::RenderConfig, RendCof)
{
    right << RendCof.NumSteps;
    right << RendCof.Sampler;
    right << (int32_t)RendCof.Vae;

    return right;

}
