#pragma once
#include "Include/Axodox.MachineLearning.h"


// Is putting this under the Axodox::MachineLearning namespace too "cargo-cult"-y?
namespace Axodox::MachineLearning
{
	class ESRGAN
	{
		static inline const Infrastructure::logger _logger{ "ESRGAN" };

	public:
		ESRGAN(OnnxEnvironment& environment, const std::string& ModelPath);

		/*
		Upscale an image (or most likely, chunk) with ESRGAN

		ImageChunk: Image tensor size (batch = 1, channels = 3, height, width), RGB normalized to 0 - 1.f
		Returns: Image tensor size (batch = 1, channnels = 3, height = input height x upscale_fac, width = input width x upscale_fac)

		*/
		Tensor Upscale(const Tensor& ImageChunk, Threading::async_operation_source* async = nullptr);

		void Evict();
	private:
		OnnxEnvironment& _environment;
		Ort::Session _session;
		bool _isUsingFloat16;
	};
}
