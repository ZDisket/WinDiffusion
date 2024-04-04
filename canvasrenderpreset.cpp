#include "canvasrenderpreset.h"

CanvasRenderPreset::CanvasRenderPreset() {}

CanvasRenderPreset::CanvasRenderPreset(const CanvasRenderPreset &pres)
{
    Preview = pres.Preview;
    Final = pres.Final;
    CFGScale = pres.CFGScale;
    Resolution = pres.Resolution;

}

ZFILE_OOVR(CanvasRenderPreset, Pres)
{
    right >> Pres.Preview;
    right >> Pres.Final;

    std::string reso;

    right >> reso; Pres.Resolution = QString::fromStdString(reso);

    right >> Pres.CFGScale;

    return right;

}

ZFILE_IOVR(CanvasRenderPreset, Pres)
{
    right << Pres.Preview;
    right << Pres.Final;

    right << Pres.Resolution.toStdString();
    right << Pres.CFGScale;

    return right;

}
