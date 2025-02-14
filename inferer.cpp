#include "inferer.h"
#include <iostream>
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


// Blend the inpainted output with the input image using a soft mask created from maskImage.
// - inputImage: the original image.
// - outputImage: the inpainted image.
// - maskImage: an ARGB mask; fully opaque pixels (alpha > ~128) are considered masked.
// - blendWidth: the width (in pixels) of the blending border.
QImage blendInpaintedRegion(const QImage &inputImage,
                            const QImage &outputImage,
                            const QImage &maskImage,
                            int blendWidth)
{
    // Ensure all images are the same size.
    const int width  = inputImage.width();
    const int height = inputImage.height();
    QImage result(inputImage.size(), inputImage.format());

    // If no blending is desired, use a hard mask.
    if (blendWidth <= 0)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                // Use a threshold of 128; adjust if needed.
                int alpha = qAlpha(maskImage.pixel(x, y));
                if (alpha > 128)
                    result.setPixel(x, y, outputImage.pixel(x, y));
                else
                    result.setPixel(x, y, inputImage.pixel(x, y));
            }
        }
        return result;
    }

    // Step 1: Create a binary soft mask from the mask's alpha channel.
    // We'll consider pixels with alpha > 128 as "1" (masked) and others as "0".
    std::vector<float> softMask(width * height, 0.0f);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int a = qAlpha(maskImage.pixel(x, y));
            softMask[y * width + x] = (a > 128 ? 1.0f : 0.0f);
        }
    }

    // Step 2: Apply a box blur to the binary soft mask to smooth the edges.
    // The box filter size is (2*blendWidth+1) x (2*blendWidth+1).
    std::vector<float> blurred(width * height, 0.0f);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float sum = 0.0f;
            int count = 0;
            for (int j = -blendWidth; j <= blendWidth; ++j)
            {
                int yy = y + j;
                if (yy < 0 || yy >= height)
                    continue;
                for (int i = -blendWidth; i <= blendWidth; ++i)
                {
                    int xx = x + i;
                    if (xx < 0 || xx >= width)
                        continue;
                    sum += softMask[yy * width + xx];
                    ++count;
                }
            }
            blurred[y * width + x] = sum / count; // value in [0,1]
        }
    }

    // Step 3: Blend outputImage and inputImage using the blurred soft mask.
    // A weight of 1.0 uses the outputImage (inpainted result); 0.0 uses the inputImage.
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float w = blurred[y * width + x];  // Blending weight

            QColor inColor  = QColor::fromRgba(inputImage.pixel(x, y));
            QColor outColor = QColor::fromRgba(outputImage.pixel(x, y));

            int r = static_cast<int>(inColor.red()   * (1.0f - w) + outColor.red()   * w);
            int g = static_cast<int>(inColor.green() * (1.0f - w) + outColor.green() * w);
            int b = static_cast<int>(inColor.blue()  * (1.0f - w) + outColor.blue()  * w);
            int a = static_cast<int>(inColor.alpha() * (1.0f - w) + outColor.alpha() * w);

            result.setPixel(x, y, qRgba(r, g, b, a));
        }
    }

    return result;
}



void Inferer::DoInference()
{

    StableDiffusionJobType CurrentJobType = StableDiffusionJobType::Txt2Img;


    // Bulk upscale jobs load input
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
            Opts.Seed = QtAxInterop::InterOpHelper::getRandomUint32();
        }


        auto Buffs = Model->DoTxt2Img(Prompt,NegativePrompt, Opts, AsyncSrc);
        if (!Buffs.size()){

            emit ThreadFinished();
            return;
        }

        QImage::Format format = QImage::Format_RGBA8888;

        for (auto& Buff : Buffs)
        {
            // the output image
            QImage image(Buff.data(), Opts.Width, Opts.Height, Opts.Width * BYTES_PER_PIXEL_RGBA, format);

            // Handle inpainting case
            if (!InputMask.isNull())
            {
                // Do these actually do anything? The image and mask should be already scaled.
                QImage scaledMask = InputMask.scaled(Opts.Width,Opts.Height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                QImage scaledImg = InputImage.scaled(Opts.Width,Opts.Height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

                image = blendInpaintedRegion(scaledImg, image, scaledMask, SmoothnessWidth);


            }


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
