#include "recommendedsettings.h"
#include <fstream>
#include "nlohmann/json.hpp"
#include <filesystem>

#define STROF(x) (#x)

using namespace std;
using namespace nlohmann;
#include <QDebug>

bool RecommendedSettings::Load(const QString &filename)
{

    filesystem::path fPath = PATH_FROM_QSTRING(filename);


    if (!filesystem::exists(fPath))
        return false;


    ifstream ifs(fPath);

    if (!ifs.good())
        throw std::runtime_error("The JSON file is empty, invalid, or does not exist (but it seems to do?)");


    // read a JSON file
    json jdat = json::parse(ifs);


    /*
     * Theoretically, I could wrap up a kind of property<name, type> type to fancy up the code and streamline copying
     * But do I want to? Nah, overengineered.
    */

    auto recommendedSettingsPart = jdat["recommended_settings"];

    resolution = QString::fromStdString(
        recommendedSettingsPart[STROF(resolution)].get<std::string>()
        );

    sampling_steps = recommendedSettingsPart[STROF(sampling_steps)].get<int32_t>();
    cfg_scale = recommendedSettingsPart[STROF(cfg_scale)].get<float>();

    sampler = QString::fromStdString(
        recommendedSettingsPart[STROF(sampler)].get<std::string>()
        );

    recommended_upscaler = QString::fromStdString(
        jdat[STROF(recommended_upscaler)].get<std::string>()
        );

    canvas_preset = QString::fromStdString(
        jdat[STROF(canvas_preset)].get<std::string>()
        );


    ifs.close();

    return true;
}

RecommendedSettings::RecommendedSettings() {}
