#pragma once
#include "Include/Axodox.Graphics.h"
#include "Include/Axodox.Collections.h"
#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.MachineLearning.h"

#include <QImage>

namespace QtAxInterop{

const int BYTES_PER_PIXEL_RGBA = 4;

/*
If I just use a namespace I get symbol already defined errors, but not if I use a class
On older versions of VC++(17) I could use just a namespace no problem for this stuff.
*/
class InterOpHelper
{
    InterOpHelper(){};

public:

    /*
     *
     * Pass-as-reference for consistency with the inverse.
    */
    static void QImageToTextureData(QImage& ReadyImage, Axodox::Graphics::TextureData& TexDat)
    {
        ReadyImage = ReadyImage.convertToFormat(QImage::Format_RGBA8888);
        TexDat.Width = ReadyImage.width(); TexDat.Height = ReadyImage.height();
        TexDat.Stride = ReadyImage.bytesPerLine();
        TexDat.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        const uint8_t* data = reinterpret_cast<const uint8_t*>(ReadyImage.bits());
        size_t dataSize = static_cast<size_t>(ReadyImage.sizeInBytes());

        TexDat.Buffer.assign(data, data + dataSize);

    }

    /*
     * Why are we using pass-as-reference instead of returning a value?
     * Because QImage, when built from a buffer, doesn't own its memory - it's just a view.
    */
    static void TextureDataToQImage(Axodox::Graphics::TextureData& TexDat, QImage& OutImg)
    {

        int bytesPerPixel = 4; // 4 for RGBA, 3 for RGB
        QImage::Format format = QImage::Format_RGBA8888;

        auto ImageBuffer = TexDat.ToFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB).Buffer; // ToBuffer() emits obscure D3D12 error because it's been shitting itself

        // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB == QImage::Format_RGBA8888
        QImage image(ImageBuffer.data(), TexDat.Width, TexDat.Height, TexDat.Width * bytesPerPixel, format);

        OutImg = image.copy();


    }


};


}
