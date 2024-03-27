#include "canvasinferer.h"
#include "QtAxodoxInteropCommon.hpp"

#include <QDebug>


using namespace Axodox::Graphics;
using namespace Axodox::MachineLearning;
using namespace Axodox::Collections;

using namespace QtAxInterop;


void CanvasInferer::ProcessOrder(CanvasOrder &Ord)
{

    QImage& InputImage = Ord.InputImage;
    QImage& InputMask = Ord.InputMask;
    StableDiffusionOptions& Opts = Ord.Options;


    qDebug() << " Scaling image";
    QImage ScaledImage = InputImage.scaled(Opts.Width,Opts.Height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    TextureData TexDat;

    InterOpHelper::QImageToTextureData(ScaledImage, TexDat);

    // Prepare settings

    // Using the Tiny VAE encoder results in too much quality deteoriation and [result deviation <- (this one especially is really bad!)
    Model->SetVaeMode(VaeMode::Normal);


    try {
        Opts.LatentInput = Model->EncodeImageVAE(TexDat);

    } catch (std::exception& Ex) {

        qDebug() << "Failed to make latent input: " << Ex.what();
    }


    if (!InputMask.isNull())
    {
        qDebug() << " Input mask!";
        QImage ScaledMask = InputMask.scaled(Opts.Width, Opts.Height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                .scaled(Opts.Width / 8, Opts.Height / 8, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        TextureData MaskTexDat;
        InterOpHelper::QImageToTextureData(ScaledMask, MaskTexDat);

        Opts.MaskInput = Tensor::FromTextureData(MaskTexDat.ToFormat(DXGI_FORMAT_R8_UNORM), ColorNormalization::LinearZeroToOne);

    }



    // Prepare settings

    Model->SetVaeMode(Ord.Vae);

    for (uint32_t i = 0; i < Ord.BatchCount;i++)
    {


        std::string Prompt = Ord.Prompt.toStdString();
        std::string NegativePrompt = Ord.NegativePrompt.toStdString();

        qDebug() << "Doing";
        auto Buffs = Model->DoTxt2Img(Prompt,NegativePrompt, Opts, AsyncSrc);

        QImage::Format format = QImage::Format_RGBA8888;

        for (auto& Buff : Buffs)
        {

            QImage image(Buff.data(), Opts.Width, Opts.Height, Opts.Width * BYTES_PER_PIXEL_RGBA, format);


            emit Done(image.copy()); // the .copy is VERY important!!!!! apparently QImage from buffer doesn't copy the data so the UI thread ends up trying to use invalid memory otherwise.
        }



    }
    qDebug() << "Done";


}

void CanvasInferer::run()
{
    qDebug() << "Canvas thread start";

    while (!Stop)
    {
        CanvasOrder Ord;

        Queue.wait_and_pop(Ord);

        qDebug() << "Order popped";

        ProcessOrder(Ord);



    }



    QThread::run();
}

CanvasInferer::CanvasInferer() {
    Stop = false;
}

void CanvasInferer::OnPreviewsAvailable(std::vector<Axodox::Graphics::TextureData> Previews)
{

    std::vector<QImage> ReEmit;
    for (auto& Img : Previews)
    {
        QImage PreviewImage;
        QtAxInterop::InterOpHelper::TextureDataToQImage(Img, PreviewImage);
        ReEmit.push_back(PreviewImage.copy());

    }
    emit PreviewsAvailable(ReEmit);

}
