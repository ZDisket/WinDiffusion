#ifndef STABLEDIFFUSIONMODEL_H
#define STABLEDIFFUSIONMODEL_H
#include "Include/Axodox.MachineLearning.h"

class StableDiffusionModel
{
private:
    std::unique_ptr<Axodox::MachineLearning::TextEmbedder> TxtEmbedder;
    std::unique_ptr<Axodox::MachineLearning::StableDiffusionInferer> UNet;
    std::unique_ptr<Axodox::MachineLearning::VaeDecoder> VAE_D;
    std::unique_ptr<Axodox::MachineLearning::VaeEncoder> VAE_E;
    std::unique_ptr<Axodox::MachineLearning::OnnxEnvironment> Env;
    ID3D12Debug* debugController;

    Axodox::MachineLearning::StableDiffusionSchedulerPredictionType PredictionType;

    bool Loaded;

    void GetPredictionType(const std::string& ModelPath);
    void CreateTextEmbeddings(const std::string& PosPrompt, const std::string& NegPrompt, Axodox::MachineLearning::StableDiffusionOptions& Options, Axodox::MachineLearning::ScheduledTensor* SchTensor);
    void LoadVAEEncoder();
public:
    StableDiffusionModel();

    void Destroy();
    bool Load(const std::string& ModelPath);

    Axodox::MachineLearning::Tensor EncodeImageVAE(const Axodox::Graphics::TextureData& TexData);

    std::vector<Axodox::Collections::aligned_vector<uint8_t>> DoTxt2Img(const std::string& Prompt, const std::string& NegativePrompt, Axodox::MachineLearning::StableDiffusionOptions Options, Axodox::Threading::async_operation_source* OpSrc = nullptr);

    void ReleaseDebugController();

    inline bool IsLoaded() const {return Loaded;}
    inline Axodox::MachineLearning::OnnxEnvironment* GetEnv() {return Env.get();}
    void LoadMinimal();

    ~StableDiffusionModel();
};

#endif // STABLEDIFFUSIONMODEL_H
