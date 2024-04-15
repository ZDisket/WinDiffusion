#ifndef CHECKGPUDIALOG_H
#define CHECKGPUDIALOG_H

#include "qlabel.h"
#include <QDialog>

namespace Ui {
class CheckGPUDialog;
}



enum class GPUThreshold{
    Good = 0,
    Okay,
    Inadequate
};


class CheckGPUDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckGPUDialog(QWidget *parent = nullptr);
    ~CheckGPUDialog();

private slots:
    void on_buttonBox_accepted();

private:
    void BuildUI();


    GPUThreshold GetThreshold(int32_t roundedVRAM, const std::vector<int32_t>& VRamThresholds);
    void SetLabelThreshold(GPUThreshold thres, const QString& modelName,QLabel& pixmapLbl, QLabel& textLabel);



    Ui::CheckGPUDialog *ui;
};

#endif // CHECKGPUDIALOG_H
