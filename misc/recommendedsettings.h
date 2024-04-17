#pragma once

#define PATH_FROM_QSTRING(str) std::filesystem::path(str.toStdString())

#include <QString>

class RecommendedSettings
{
public:
    QString resolution;
    int32_t sampling_steps;
    float cfg_scale;
    QString sampler;

    QString recommended_upscaler;
    QString canvas_preset;


    bool Load(const QString& filename);




    RecommendedSettings();
};

