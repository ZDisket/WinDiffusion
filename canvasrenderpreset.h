#pragma once

#include "renderconfigform.h"






class CanvasRenderPreset
{
public:
    RenderConf::RenderConfig Preview;
    RenderConf::RenderConfig Final;

    QString Resolution;
    double CFGScale;


    CanvasRenderPreset();

    CanvasRenderPreset(const CanvasRenderPreset& pres);
};

ZFILE_IOVR(CanvasRenderPreset, Pres);

ZFILE_OOVR(CanvasRenderPreset, Pres);
