#ifndef INFERER_H
#define INFERER_H

#include <QThread>
#include <QListWidgetItem>
#include "stablediffusionmodel.h"
#include "upscaler.h"
#include <QImage>

enum class StableDiffusionJobType{
  Txt2Img,
  Img2Img,
  Upscale,
  BulkUpscale,
  Canvas
};
// An Voxer is a thread spawned for the sole purpose of doing inference
class Inferer : public QThread
{
    Q_OBJECT

    void run() override;



public:
    Inferer();

    void DoInference();

    StableDiffusionModel* Model;
    Upscaler* EsrGan;

    Axodox::MachineLearning::StableDiffusionOptions Opts;
    Axodox::Threading::async_operation_source* AsyncSrc;

    QImage InputImage;
    QImage InputMask;
    std::string Prompt;
    std::string NegativePrompt;
    uint32_t BatchCount;
    bool RandomSeed;

    std::string OutputPath;
    QListWidgetItem* itmInput;

    int32_t GetStepsDone();


signals:
    void DoneBulk(QImage Img, std::string OutputPath, QListWidgetItem* Itm);
    void Done(QImage Img, StableDiffusionJobType JobType);
    void ThreadFinished();
    void PreviewsAvailable(std::vector<QImage> Prevs);

public slots:
    void OnPreviewsAvailable(std::vector<Axodox::Graphics::TextureData> Previews);

};

#endif // INFERER_H
