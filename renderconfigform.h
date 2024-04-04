#ifndef RENDERCONFIGFORM_H
#define RENDERCONFIGFORM_H

#include <QWidget>
#include "ext/ZFile.h"

namespace Ui {
class RenderConfigForm;
}

namespace RenderConf{

enum VaeSel{
  Normal = 0,
  Tiny
};

struct RenderConfig{
    uint32_t NumSteps;
    int32_t Sampler;
    VaeSel Vae;
};



};


ZFILE_IOVR(RenderConf::RenderConfig, RendCof);


ZFILE_OOVR(RenderConf::RenderConfig, RendCof);


class RenderConfigForm : public QWidget
{
    Q_OBJECT

public:
    explicit RenderConfigForm(QWidget *parent = nullptr);
    ~RenderConfigForm();


    RenderConf::RenderConfig GetConfig();
    void SetConfig(const RenderConf::RenderConfig& InConf);

private:
    Ui::RenderConfigForm *ui;
};

#endif // RENDERCONFIGFORM_H
