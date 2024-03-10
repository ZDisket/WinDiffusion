#include "stablediffusionmodel.h"
#include "Collections/Hasher.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/DependencyContainer.h"


using namespace Axodox::Graphics;
using namespace Axodox::MachineLearning;
using namespace Axodox::Collections;
#include "nlohmann/json.hpp"
#include <fstream>
using json = nlohmann::json;
using namespace std;



void StableDiffusionModel::GetPredictionType(const std::string &ModelPath)
{
    std::string SchedulerConfPath = "scheduler/scheduler_config.json";


    if (!(ModelPath[ModelPath.size() - 1] == '/' || ModelPath[ModelPath.size() - 1] == '\\'))
        SchedulerConfPath = "/" + SchedulerConfPath;

    // read a JSON file
    std::ifstream i(ModelPath + SchedulerConfPath);

    json SchedulerConf;

    i >> SchedulerConf;

    i.close();

    std::string PredTypeString = SchedulerConf["prediction_type"].get<std::string>();

    if (PredTypeString == "v_prediction")
        PredictionType = StableDiffusionSchedulerPredictionType::V;
    else
        PredictionType = StableDiffusionSchedulerPredictionType::Epsilon;



}

void StableDiffusionModel::CreateTextEmbeddings(const std::string &PosPrompt, const std::string &NegPrompt, Axodox::MachineLearning::StableDiffusionOptions &Options, ScheduledTensor *SchTensor)
{



    auto encodedNegativePrompt = TxtEmbedder->SchedulePrompt(NegPrompt, Options.StepCount);
    auto encodedPositivePrompt = TxtEmbedder->SchedulePrompt(PosPrompt, Options.StepCount);

    Options.TextEmbeddings.Weights.reserve(encodedNegativePrompt[0].Weights.size() + encodedPositivePrompt[0].Weights.size());
    for (auto weight : encodedNegativePrompt[0].Weights) Options.TextEmbeddings.Weights.push_back(-weight);
    for (auto weight : encodedPositivePrompt[0].Weights) Options.TextEmbeddings.Weights.push_back(weight);

    ScheduledTensor tensor = *SchTensor;
    trivial_map<pair<void*, void*>, shared_ptr<EncodedText>> embeddingBuffer;
    for (auto i = 0u; i < Options.StepCount; i++)
    {
        auto& concatenatedTensor = embeddingBuffer[{ encodedNegativePrompt[i].Tensor.get(), encodedPositivePrompt[i].Tensor.get() }];
        if (!concatenatedTensor)
        {
            concatenatedTensor = make_shared<EncodedText>(encodedNegativePrompt[i].Tensor->Concat(*encodedPositivePrompt[i].Tensor));
        }

        tensor[i] = concatenatedTensor;
    }

    Options.TextEmbeddings.Tensor = tensor;

}

void StableDiffusionModel::LoadVAEEncoder()
{
    VAE_E = std::make_unique<VaeEncoder>(*Env);

}

StableDiffusionModel::StableDiffusionModel() {
    Loaded = false;
/*
    debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();

    }
    if (debugController) debugController->Release();
*/
}

void StableDiffusionModel::Destroy()
{
    try
    {
        Env.reset();
        TxtEmbedder.reset();
        UNet.reset();
        VAE_D.reset();
        VAE_E.reset();

    }catch (...)
    { // nobody gives a shit about errors on deletion.

    }

}

bool StableDiffusionModel::Load(const std::string &ModelPath)
{
    if (Loaded)
        Destroy();

    if (Env)
        Env.reset();

    Env = std::make_unique<OnnxEnvironment>(ModelPath);

    TxtEmbedder = std::make_unique<TextEmbedder>(*Env);
    UNet = std::make_unique<StableDiffusionInferer>(*Env);
    VAE_D = std::make_unique<VaeDecoder>(*Env);

    GetPredictionType(ModelPath);
    Loaded = true;

    return true;
}

Tensor StableDiffusionModel::EncodeImageVAE(const Axodox::Graphics::TextureData& TexData)
{

    if (!VAE_E)
        LoadVAEEncoder();


    Tensor InpTexTens = Tensor::FromTextureData(TexData.ToFormat(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB), ColorNormalization::LinearPlusMinusOne);
    return VAE_E->EncodeVae(InpTexTens);

}

std::vector<Axodox::Collections::aligned_vector<uint8_t>> StableDiffusionModel::DoTxt2Img(const std::string &Prompt, const std::string &NegativePrompt, Axodox::MachineLearning::StableDiffusionOptions Options, Axodox::Threading::async_operation_source *OpSrc)
{
    // Make embeddings
    Options.PredictionType = PredictionType;

    ScheduledTensor ScheduledEmbedTens{Options.StepCount};
    std::vector<aligned_vector<uint8_t>> ImageBuffers;

    CreateTextEmbeddings(Prompt, NegativePrompt, Options, &ScheduledEmbedTens);

    // Inference UNet

    auto x = UNet->RunInference(Options, OpSrc);

    if (OpSrc->is_cancelled())
        return std::vector<aligned_vector<uint8_t>>{};

    // VAE

    x = VAE_D->DecodeVae(x);

    TextureData d;

    auto ImageTextures = x.ToTextureData(ColorNormalization::LinearPlusMinusOne);

    for (auto& ImgTexture : ImageTextures ){
        auto ImageBuffer = ImgTexture.ToFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB).Buffer; // ToBuffer() emits obscure D3D12 error because it's been shitting itself
        ImageBuffers.push_back(ImageBuffer);
    }



    return ImageBuffers;

}

void StableDiffusionModel::ReleaseDebugController()
{
    if (debugController) debugController->Release();

}

void StableDiffusionModel::LoadMinimal()
{
    Env = std::make_unique<OnnxEnvironment>("");
}

StableDiffusionModel::~StableDiffusionModel()
{

    Destroy();

}
