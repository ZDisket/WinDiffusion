# WinDiffusion




![](demo/demoreel.png)

*Made with RealVisXL Lightning, 8 steps Euler A and upscaled with 4xUltraSharp*

## What is this?

WinDiffusion is a Stable Diffusion frontend written in C++/Qt, without a single line of Python involved, using the ONNX runtime and DirectML to execute models

### So, what's the deal?

1. **Natively compatible with all GPU vendors**. The DirectML backend supports any DirectX 12-capable GPU
2. **Lightweight**. Everything needed to run the models is ~200MB, compared to the around 10GB of pip or conda-installed libraries.
3. **Easy to install**. Installation is a breeze—simply unzip and launch the executable. It's so simple, even your grandma could do it.
4. **Self-contained, reliable**. Without having to lug around lots of libraries, it remains unaffected by unforeseen changes in dependencies.

## Support

Marked with ❌ means not currently available, but is on high priority.

### Supported models (tested)

- ✔️ Stable Diffusion 1.5
- ✔️ Stable Diffusion XL
- ✔️ Stable Diffusion XL Turbo
- ✔️ Stable Diffusion XL Lightning

### Samplers
- ✔️ DPM 2M++ Karras
- ✔️ Euler Ancestral
- ❌ DPM++ SDE Karras (for models that demand it, use Euler Ancestral instead for now)

### Features
- ✔️ Text-to-image
- ✔️ Image-to-image
- ✔️ Inpainting
- ✔️ Upscaling with ESRGAN
- ❌ Prompt ((weighting))
- ❌ Long prompts (longer than CLIP limit)
- ❌ Face fix

## Compilation

TODO: fill out this section

## Externals (and thanks)

- [Axodox-machinelearning](https://github.com/axodox/axodox-machinelearning): C++ implementation of Stable Diffusion
- [QGoodWindow](https://github.com/antonypro/QGoodWindow): Fancy windows for Qt
- [JSON](https://github.com/nlohmann/json)


