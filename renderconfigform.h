#ifndef RENDERCONFIGFORM_H
#define RENDERCONFIGFORM_H

#include <QWidget>

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
    QString Sampler;
    VaeSel Vae;
};



};



class RenderConfigForm : public QWidget
{
    Q_OBJECT

public:
    explicit RenderConfigForm(QWidget *parent = nullptr);
    ~RenderConfigForm();


    RenderConf::RenderConfig GetConfig();

private:
    Ui::RenderConfigForm *ui;
};

#endif // RENDERCONFIGFORM_H
