#ifndef UPSCALER_H
#define UPSCALER_H
#include "ESRGAN.h"
#include <QImage>

class Upscaler
{
private:
    Axodox::MachineLearning::OnnxEnvironment* Env;
    std::unique_ptr<Axodox::MachineLearning::ESRGAN> Model;
    bool Loaded;

    void Destroy();
    QList<QImage> SplitImageIntoChunks(const QImage &image, int chunkSize = 128, int overlap = 48);
    QImage RemoveOverlapFromChunk(const QImage &chunk, const QSize &finalChunkSize, const QSize &originalSize, int overlap, int xPosition, int yPosition);
    QImage JoinImageChunks(const QList<QImage> &chunks, const QSize &finalChunkSize, const QSize &originalSize, int overlap);
public:
    Upscaler();

    void Load(const std::string& ModelPath);

    /*
     * Upscale a whole image with ESRGAN
     *
     * InImg -> Image input
     * TileSize -> Size, in pixels, of each tile. We split into tiles because anything over 256x256 would require like 32GB of VRAM
     * Overlap -> Overlap, in pixels of each tile, to eliminate tiling artifacts.
     *
     * Returns:
     * <- Upscaled image. Consult the model you're using for the factor. Most are 4x, some 2x, even 8x.
     *
    */
    QImage UpscaleImg(const QImage& InImg, uint32_t TileSize, uint32_t Overlap = 48, Axodox::Threading::async_operation_source* async_src = nullptr);

    bool IsLoaded(){return Loaded;}
    void SetEnv(Axodox::MachineLearning::OnnxEnvironment* InEnv){Env = InEnv;}
};

#endif // UPSCALER_H
