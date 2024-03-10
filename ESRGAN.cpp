#include "ESRGAN.h"
using namespace Axodox::Infrastructure;
using namespace Ort;
using namespace std;

namespace Axodox::MachineLearning 
{
    Axodox::MachineLearning::ESRGAN::ESRGAN(OnnxEnvironment& environment, const std::string& ModelPath) :
        _environment(environment),
        _session(environment->CreateSession(ModelPath))
    {

        auto metadata = OnnxModelMetadata::Create(_environment, _session);

        _session.Evict();
        _logger.log(log_severity::information, "Loaded.");
        _isUsingFloat16 = false;
    }

    Tensor ESRGAN::Upscale(const Tensor& ImageChunk, Threading::async_operation_source* async)
    {
        //Bind values
        IoBinding bindings{ _session };
        bindings.BindInput("input_img", ImageChunk.ToOrtValue());
        bindings.BindOutput("upscaled", _environment->MemoryInfo());

        //Run inference
        _session.Run({}, bindings);

        //Get result
        auto outputValues = bindings.GetOutputValues();
        auto result = Tensor::FromOrtValue(outputValues[0]).ToSingle();

        return result;


    }

    void ESRGAN::Evict()
    {
        _session.Evict();
    }

}
