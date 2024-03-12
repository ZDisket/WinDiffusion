#ifndef STABLEDIFFUSIONMODEL_H
#define STABLEDIFFUSIONMODEL_H
#include "Include/Axodox.MachineLearning.h"
#include <QObject>
class StableDiffusionModel : public QObject // We only use QObject for the signals, the class is made to be portable.
{
    Q_OBJECT

private:
    std::unique_ptr<Axodox::MachineLearning::TextEmbedder> TxtEmbedder;
    std::unique_ptr<Axodox::MachineLearning::StableDiffusionInferer> UNet;
    std::unique_ptr<Axodox::MachineLearning::VaeDecoder> VAE_D;
    std::unique_ptr<Axodox::MachineLearning::VaeEncoder> VAE_E;
    std::unique_ptr<Axodox::MachineLearning::VaeDecoder> VAE_D_Tiny;
    std::unique_ptr<Axodox::MachineLearning::OnnxEnvironment> Env;
    ID3D12Debug* debugController;

    Axodox::MachineLearning::StableDiffusionSchedulerPredictionType PredictionType;

    bool Loaded;

    void GetPredictionType(const std::string& ModelPath);
    void CreateTextEmbeddings(const std::string& PosPrompt, const std::string& NegPrompt, Axodox::MachineLearning::StableDiffusionOptions& Options, Axodox::MachineLearning::ScheduledTensor* SchTensor);

    Axodox::MachineLearning::Tensor RunInference( Axodox::MachineLearning::StableDiffusionOptions& Options, Axodox::Threading::async_operation_source* OpSrc = nullptr);

    void LoadVAEEncoder();
public:
    StableDiffusionModel();

    void Destroy();
    bool Load(const std::string& ModelPath, const std::string &AuxiliaryPath);

    Axodox::MachineLearning::Tensor EncodeImageVAE(const Axodox::Graphics::TextureData& TexData);

    std::vector<Axodox::Collections::aligned_vector<uint8_t>> DoTxt2Img(const std::string& Prompt, const std::string& NegativePrompt, Axodox::MachineLearning::StableDiffusionOptions Options, Axodox::Threading::async_operation_source* OpSrc = nullptr);

    void ReleaseDebugController();

    inline bool IsLoaded() const {return Loaded;}
    inline Axodox::MachineLearning::OnnxEnvironment* GetEnv() {return Env.get();}
    void LoadMinimal();

    ~StableDiffusionModel();

signals:
    void PreviewAvailable(std::vector<Axodox::Graphics::TextureData> Prevs);
};

#endif // STABLEDIFFUSIONMODEL_H
