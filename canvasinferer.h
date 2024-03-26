#ifndef CANVASINFERER_H
#define CANVASINFERER_H

#include <QThread>
#include <QObject>
#include <QImage>
#include "threadsafequeue.hpp"
#include "Include/Axodox.MachineLearning.h"
#include "stablediffusionmodel.h"


struct CanvasOrder{
    QString Prompt;
    QString NegativePrompt;
    Axodox::MachineLearning::StableDiffusionOptions Options;
    uint32_t BatchCount;
    QImage InputImage;
    QImage InputMask;
    VaeMode Vae;


};



/*
 * CanvasInferer is the dedicated inference thread class for Canvas
 * Unlike its normal Inferer counterpart, this one:
 * - Always runs
 * - Uses a thread-safe queue and constantly processes for its orders
 * - Can only do img2img.
*/
class CanvasInferer : public QThread
{
    Q_OBJECT

private:
    void ProcessOrder(CanvasOrder& Ord);
protected:
    void run() override;

public:
    CanvasInferer();

    Axodox::Threading::async_operation_source* AsyncSrc;
    StableDiffusionModel* Model;
    bool Stop;


    ThreadSafeQueue<CanvasOrder> Queue;

signals:
    void Done(QImage Img);
    void PreviewsAvailable(std::vector<QImage> Imgs);

public slots:
    void OnPreviewsAvailable(std::vector<Axodox::Graphics::TextureData> Previews);
};

#endif // CANVASINFERER_H
