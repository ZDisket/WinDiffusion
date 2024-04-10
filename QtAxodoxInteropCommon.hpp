#pragma once
#include "Include/Axodox.Graphics.h"
#include "Include/Axodox.Collections.h"
#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.MachineLearning.h"
#include <random>
#include <QImage>


#define IF_EXCEPT(cond, ex) if (cond){throw ex;}

namespace QtAxInterop{

using SDSchedulerT = Axodox::MachineLearning::StableDiffusionSchedulerKind;
const int BYTES_PER_PIXEL_RGBA = 4;



/*
If I just use a namespace I get symbol already defined errors, but not if I use a class
On older versions of VC++(17) I could use just a namespace no problem for this stuff.
*/
class InterOpHelper
{
    InterOpHelper(){};

public:

    template<typename T>
    static T getRandomNum()
    {
        std::random_device rd;

        std::mt19937 gen(rd());

        std::uniform_int_distribution<T> _rand_distrub{};

        return _rand_distrub(gen);
    }

    static uint32_t getRandomUint32() {
        // Create a random device
        std::random_device rd;

        // Use the random device to seed a Mersenne Twister engine
        std::mt19937 gen(rd());

        // Define a distribution range. In this case, it's the full range of uint32_t
        std::uniform_int_distribution<uint32_t> _rand_distrub{};

        // Generate and return a random uint32_t
        return _rand_distrub(gen);
    }


    static std::vector<SDSchedulerT> ComboBoxIDToScheduler(){
        return std::vector<SDSchedulerT>{SDSchedulerT::DpmPlusPlus2M , SDSchedulerT::EulerAncestral};
        };


        // Convert a 0-100 int slider range to 0 - 1.f float range.
        static float SliderToZeroOneRange(int slival) {return ((float)slival) / 100.f;};

        // Convert a 0 - 1.f float range to 0-100 int slider range.
        static int   ZeroOneToSliderRange(float frange) {return (int)(frange * 100.f);};


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

        QImage::Format format = QImage::Format_RGBA8888;

        auto ImageBuffer = TexDat.ToFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB).Buffer; // ToBuffer() emits obscure D3D12 error because it's been shitting itself

        // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB == QImage::Format_RGBA8888
        QImage image(ImageBuffer.data(), TexDat.Width, TexDat.Height, TexDat.Width * BYTES_PER_PIXEL_RGBA, format);

        OutImg = image.copy();


    }

    /*
     * Preprocess a Stable Diffusion prompt with ((emphasis)) tags into (emphasis:1.2) tags, turning nested parenthesis into + 0.1 emphasis for each nest level
     * For example, "an (important) word and ((very important)) point" -> "an (important:1.1) word and (very important:1.2) point"
     * Thanks GPT-4
    */
    static QString PreprocessPrompt(QString input) {
        QString result = input;
        QList<int> openParensPositions;

        // First pass: Adjust emphasis tags with weights
        for (int i = 0; i < result.length(); ++i) {
            if (result[i] == '(') {
                openParensPositions.push_back(i);
            } else if (result[i] == ')' && !openParensPositions.isEmpty()) {
                int lastOpenPos = openParensPositions.takeLast();

                // Calculate emphasis level based on the depth of this set of parentheses
                float emphasisLevel = 1.0 + 0.1 * (openParensPositions.size() + 1);

                // Check if content inside is an emphasis tag without a number
                QString content = result.mid(lastOpenPos + 1, i - lastOpenPos - 1);
                if (!content.contains(":")) {
                    // It's an emphasis tag without a number, adjust it
                    QString replacement = QString(":%1").arg(QString::number(emphasisLevel, 'f', 1));
                    result.insert(i, replacement);
                    i += replacement.length(); // Adjust current index to account for the inserted text
                }
            }
        }

        // Second pass: Remove nested parentheses
        for (int i = 1; i < result.length(); ) { // Start from 1 to safely check previous character
            if (result[i] == '(' && result[i - 1] == '(') {
                result.remove(i, 1); // Remove the extra '('
                continue; // Do not increment i to recheck at the same position
            } else if (result[i] == ')' && i < result.length() - 1 && result[i + 1] == ')') {
                result.remove(i, 1); // Remove the extra ')'
                // Do not increment i to potentially handle multiple consecutive ')' removal
                continue;
            } else {
                i++; // Only increment if no removals were made
            }
        }

        return result;
    }


};


}
