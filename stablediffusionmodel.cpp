#include "stablediffusionmodel.h"
#include "Collections/Hasher.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/DependencyContainer.h"
#include "qdebug.h"


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

    for (auto weight : Options.TextEmbeddings.Weights)
        qDebug() << weight;

}

Tensor StableDiffusionModel::RunInference(Axodox::MachineLearning::StableDiffusionOptions &Options, Axodox::Threading::async_operation_source *OpSrc)
{


    Tensor image;

    // All this to account for img2img.
    int realInitialStep = std::clamp(int(Options.StepCount - Options.StepCount * Options.DenoisingStrength - 1), 0, int(Options.StepCount));
    int totalSteps = Options.StepCount;

    int divTotalSteps = Options.StepCount - realInitialStep;
    int stepJump = std::max(divTotalSteps / 8, 1);
    int currentStep = realInitialStep + stepJump;


    Tensor* currentRawLatents = nullptr;
    Tensor rawLatents;

    for (int i = currentStep; i < totalSteps; i += stepJump)
    {

        bool isLastStep = (i + stepJump >= totalSteps);
        size_t initialStep = i - stepJump;

        rawLatents = UNet->Iterate(Options, OpSrc, currentRawLatents, initialStep, isLastStep ? totalSteps : i); // last ternary operator to account for if not divisible

        if (OpSrc->is_cancelled())
            return Tensor{};

        currentRawLatents = &rawLatents;


        if (!isLastStep)
        {
            Tensor previewImage = VAE_D_Tiny->DecodeVae(
                UNet->FinishInference(Options, rawLatents, false, true)
                );

            auto previewTextures = previewImage.ToTextureData(ColorNormalization::LinearZeroToOne); // TAESD is [0 - 1]

            emit PreviewAvailable(previewTextures);

        }


    }

    image = UNet->FinishInference(Options, rawLatents, true, CurrentVaeMode == VaeMode::Tiny);

    if (CurrentVaeMode == VaeMode::Normal)
        image = VAE_D->DecodeVae(image);
    else
        image = VAE_D_Tiny->DecodeVae(image);


    return image;
}

void StableDiffusionModel::LoadVAEEncoder(bool TinyToo)
{
    VAE_E = std::make_unique<VaeEncoder>(*Env);

    if (TinyToo)
        VAE_E_Tiny = std::make_unique<VaeEncoder>(*Env, FullTinyEncoderPath);


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
    if (!IsLoaded())
        return;

    try
    {
        //Env.reset();
        TxtEmbedder.reset();
        UNet.reset();
        VAE_D.reset();
        VAE_E.reset();
        VAE_D_Tiny.reset();

    }catch (...)
    { // nobody gives a shit about errors on deletion.

    }
    Loaded = false;

}

// AuxiliaryPath ends with "/".
bool StableDiffusionModel::Load(const std::string &ModelPath, const std::string& AuxiliaryPath)
{
    if (Loaded)
        Destroy();

    if (Env)
        Env.reset();

    Env = std::make_unique<OnnxEnvironment>(ModelPath);

    bool isSDXL = std::filesystem::is_directory(Env->RootPath() / "text_encoder_2");
    std::string TinyDecoderFn = isSDXL ? "taesdxl_decoder.onnx" : "taesd_decoder.onnx";
    std::string TinyEncoderFn = isSDXL ? "taesdxl_encoder.onnx" : "taesd_encoder.onnx";

    TxtEmbedder = std::make_unique<TextEmbedder>(*Env, AuxiliaryPath + "text_tokenizer");
    UNet = std::make_unique<StableDiffusionInferer>(*Env);
    VAE_D = std::make_unique<VaeDecoder>(*Env);
    VAE_D_Tiny = std::make_unique<VaeDecoder>(*Env, AuxiliaryPath + TinyDecoderFn);

    // Unlike the tiny decoder, the tiny encoder's name is merely saved to be loaded on-demand.
    FullTinyEncoderPath = AuxiliaryPath + TinyEncoderFn;


    CurrentVaeMode = VaeMode::Normal;



    GetPredictionType(ModelPath);
    Loaded = true;

    return true;
}

Tensor StableDiffusionModel::EncodeImageVAE(const Axodox::Graphics::TextureData& TexData)
{
    bool useTinyVae = CurrentVaeMode == VaeMode::Tiny;

    if ((!VAE_E) || (useTinyVae && !VAE_E_Tiny))
        LoadVAEEncoder(useTinyVae);


    Tensor InpTexTens = Tensor::FromTextureData(TexData.ToFormat(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB),
                                                useTinyVae ? ColorNormalization::LinearZeroToOne : ColorNormalization::LinearPlusMinusOne);

    if (!useTinyVae)
        return VAE_E->EncodeVae(InpTexTens);
    else
        return VAE_E_Tiny->EncodeVae(InpTexTens);


}

std::vector<Axodox::Collections::aligned_vector<uint8_t>> StableDiffusionModel::DoTxt2Img(const std::string &Prompt, const std::string &NegativePrompt, Axodox::MachineLearning::StableDiffusionOptions Options, Axodox::Threading::async_operation_source *OpSrc)
{
    // Make embeddings
    Options.PredictionType = PredictionType;

    ScheduledTensor ScheduledEmbedTens{Options.StepCount};
    std::vector<aligned_vector<uint8_t>> ImageBuffers;

    CreateTextEmbeddings(Prompt, NegativePrompt, Options, &ScheduledEmbedTens);

    // Inference UNet

    auto x = RunInference(Options, OpSrc);


    if (OpSrc->is_cancelled())
        return std::vector<aligned_vector<uint8_t>>{};

    TextureData d;

    auto ImageTextures = x.ToTextureData(  // account for TAESD being different
        CurrentVaeMode == VaeMode::Normal ? ColorNormalization::LinearPlusMinusOne : ColorNormalization::LinearZeroToOne
     );

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
