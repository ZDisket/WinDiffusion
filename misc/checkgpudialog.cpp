#include "checkgpudialog.h"
#include "ui_checkgpudialog.h"

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <iostream>
#include <string>
#include <utility>  // For std::pair
#include <QDebug>
#include <QMessageBox>


const QStringList thresholdTooltips =
{
    "Your GPU can run %1 models well and with memory to spare. :D",
    "Your GPU has just enough memory to run %1 models, as long as you close everything else that consumes VRAM, like games, recording software, and web browsers with lots of tabs.",
    "Your GPU doesn't have enough memory to run %1 models. Sorry :("
};
const QStringList thresholdPixmaps = {":/res/check.png", ":/res/yellowcheck.png", ":/res/cross.png"};
const QStringList thresholdLabels = {"Good!", "Okay", "Inadequate"};
const std::vector<int32_t> thresholdsSDXL = { 13, 11};
const std::vector<int32_t> thresholdsSD = {11, 7};

std::pair<std::wstring, double> GetMainAdapterInfo() {
    IDXGIFactory4* pFactory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pFactory)))) {
        std::cerr << "Failed to create DXGI Factory." << std::endl;
        return std::make_pair(L"", 0.0);
    }

    IDXGIAdapter1* pAdapter = nullptr;
    std::wstring adapterName = L"";
    SIZE_T maxDedicatedVideoMemory = 0;
    double vramGB = 0.0;

    // Enumerate adapters
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &pAdapter); ++adapterIndex) {
        DXGI_ADAPTER_DESC1 desc;
        if (SUCCEEDED(pAdapter->GetDesc1(&desc))) {
            // Check if this adapter has the most memory so far
            if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory) {
                maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
                adapterName = desc.Description;
                vramGB = static_cast<double>(maxDedicatedVideoMemory) / (1024.0 * 1024.0 * 1024.0);
            }
        }
        pAdapter->Release();
    }

    pFactory->Release();

    return std::make_pair(adapterName, vramGB);
}




CheckGPUDialog::CheckGPUDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CheckGPUDialog)
{
    ui->setupUi(this);

    BuildUI();
}

CheckGPUDialog::~CheckGPUDialog()
{
    delete ui;
}

void CheckGPUDialog::BuildUI()
{


    auto [gpuName, vramInGB] = GetMainAdapterInfo();

    if (!gpuName.size())
    {
        QMessageBox::critical(this, "Critical error!", "Error: Could not detect any DirectX devices (failed to create DXGI factory)."
                                                       "\nTroubleshooting:\n1. Do you have any GPUs?\n2. Do you have DirectX 12 installed? (You should!)\n3. Is this a physical system? (not a virtual machine)"
                                                       "\nIf the answer is yes to all, open an issue with your dxdiag information.");
        close();

        return;

    }

    ui->lblGPU->setText("Main GPU: " + QString::fromStdWString(gpuName));
    ui->lblDedicatedVRAM->setText("Dedicated video memory: " + QString::number(vramInGB,'f',1) + " GB");

    qDebug() << vramInGB;

    int32_t rVram = (int32_t)round(vramInGB);

    GPUThreshold StableDiff  = GetThreshold(rVram, thresholdsSD);
    GPUThreshold SDXL = GetThreshold(rVram, thresholdsSDXL);

    SetLabelThreshold(StableDiff, "Stable Diffusion", *ui->lblImgStableDiff, *ui->lblStableDiffStatus);
    SetLabelThreshold(SDXL, "Stable Diffusion XL", *ui->lblImgSDXL, *ui->lblSDXLStatus);


    QFont mainLblFont = ui->lblGPU->font();
    mainLblFont.setPointSize(mainLblFont.pointSize() + 1);
    ui->lblGPU->setFont(
        mainLblFont
        );







}
/*
    NOTE: size of vector must always be the number of elements in the GPUThreshold enum - 1
*/
GPUThreshold CheckGPUDialog::GetThreshold(int32_t roundedVRAM,const std::vector<int32_t> &VRamThresholds)
{
    for (size_t i = 0; i < VRamThresholds.size(); i++)
    {
        if (roundedVRAM > VRamThresholds[i])
            return (GPUThreshold)i;


    }

    return GPUThreshold::Inadequate;

}

void CheckGPUDialog::SetLabelThreshold(GPUThreshold thres,  const QString& modelName, QLabel &pixmapLbl, QLabel &textLabel)
{
    const size_t i = (size_t)thres;

    pixmapLbl.setPixmap(
        QPixmap(thresholdPixmaps[i]).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );

    textLabel.setText(modelName + ": " + thresholdLabels[i]);

    textLabel.setToolTip(thresholdTooltips[i].arg(modelName));


}

void CheckGPUDialog::on_buttonBox_accepted()
{
    close();
}

