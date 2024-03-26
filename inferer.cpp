#include "inferer.h"
#include <iostream>
#include <random>
#include <QDebug>
#include "QtAxodoxInteropCommon.hpp"
#include "pathwidgetitem.h"

using namespace Axodox::Graphics;
using namespace Axodox::MachineLearning;
using namespace Axodox::Collections;

using namespace QtAxInterop;
void Inferer::run()
{
    DoInference();
}

Inferer::Inferer() {
    AsyncSrc = nullptr;
    EsrGan = nullptr;
    Model = nullptr;
}

uint32_t getRandomUint32() {
    // Create a random device
    std::random_device rd;

    // Use the random device to seed a Mersenne Twister engine
    std::mt19937 gen(rd());

    // Define a distribution range. In this case, it's the full range of uint32_t
    std::uniform_int_distribution<uint32_t> _rand_distrub{};

    // Generate and return a random uint32_t
    return _rand_distrub(gen);
}



void Inferer::DoInference()
{

    StableDiffusionJobType CurrentJobType = StableDiffusionJobType::Txt2Img;


    // Bulk upscale jobs load input and save outputs.
    if (!OutputPath.empty())
    {
        auto pathItem = (PathWidgetItem*)itmInput;

        InputImage = QImage(pathItem->getFullPath());


    }


    if (EsrGan)
    {

        CurrentJobType = StableDiffusionJobType::Upscale;

        // If the model isn't loaded, we can spare a little more VRAM in the form of bigger tiles.
        // TODO: actually query the device for available VRAM instead of depending on whether the model is loaded or not.
        uint32_t upsTileSz = !Model->IsLoaded() ? 384 : 256;



        QImage  UpsImg = EsrGan->UpscaleImg(InputImage, upsTileSz, 48, AsyncSrc);

        if (OutputPath.empty())
            emit Done(UpsImg.copy(), CurrentJobType);
        else
        {
            UpsImg.save(QString::fromStdString(OutputPath));
            emit DoneBulk(UpsImg.copy(), OutputPath, itmInput);

        }


        emit ThreadFinished();
        return;
    }


    if (!InputImage.isNull())
    {
        CurrentJobType = StableDiffusionJobType::Img2Img;

        QImage ScaledImage = InputImage.scaled(Opts.Width,Opts.Height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        TextureData TexDat;

        InterOpHelper::QImageToTextureData(ScaledImage, TexDat);


        try {
            Opts.LatentInput = Model->EncodeImageVAE(TexDat);

        } catch (std::exception& Ex) {

            qDebug() << "Failed to make latent input: " << Ex.what();
        }


        if (!InputMask.isNull())
        {
            QImage ScaledMask = InputMask.scaled(Opts.Width, Opts.Height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                .scaled(Opts.Width / 8, Opts.Height / 8, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            TextureData MaskTexDat;
            InterOpHelper::QImageToTextureData(ScaledMask, MaskTexDat);

            Opts.MaskInput = Tensor::FromTextureData(MaskTexDat.ToFormat(DXGI_FORMAT_R8_UNORM), ColorNormalization::LinearZeroToOne);

        }

    }




    for (uint32_t i = 0; i < BatchCount;i++)
    {

        if (RandomSeed){
            Opts.Seed = getRandomUint32();
        }


        auto Buffs = Model->DoTxt2Img(Prompt,NegativePrompt, Opts, AsyncSrc);
        if (!Buffs.size()){

            emit ThreadFinished();
            return;
        }

        QImage::Format format = QImage::Format_RGBA8888;

        for (auto& Buff : Buffs)
        {

            QImage image(Buff.data(), Opts.Width, Opts.Height, Opts.Width * BYTES_PER_PIXEL_RGBA, format);


            emit Done(image.copy(), CurrentJobType); // the .copy is VERY important!!!!! apparently QImage from buffer doesn't copy the data so the UI thread ends up trying to use invalid memory otherwise.
        }



    }

    emit ThreadFinished();

}

int32_t Inferer::GetStepsDone()
{

    if (AsyncSrc)
        return ((int32_t)std::round(AsyncSrc->state().progress)) * 100;

    return -1;
}

void Inferer::OnPreviewsAvailable(std::vector<Axodox::Graphics::TextureData> Previews)
{

    std::vector<QImage> ReEmit;
    for (auto& Img : Previews)
    {
        QImage PreviewImage;
        QtAxInterop::InterOpHelper::TextureDataToQImage(Img, PreviewImage);
        ReEmit.push_back(PreviewImage);

    }
    emit PreviewsAvailable(ReEmit);


}
