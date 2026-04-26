#include "GameDataLoader.h"


#include <fstream>

namespace
{
    auto loadJsonFile(const std::string& filePath)
    {
        std::ifstream file(filePath);

        if (!file.is_open())
        {
            throw std::runtime_error("Could not open '" + filePath + "'");
        }

        return file;
    }

    nlohmann::json loadJsonFromFile(const std::string& filePath)
    {
        auto file = loadJsonFile(filePath);
        return nlohmann::json::parse(file);
    }
}


nlohmann::json GameDataLoader::loadStrings()
{
	auto jsonData = loadJsonFromFile("res/strings.json");
    return jsonData;
}


std::array<std::string, 12> GameDataLoader::loadMonthStrings()
{
    auto data = loadJsonFromFile("res/strings.json");

    if (!data.contains("strings") || !data["strings"].contains("months"))
    {
        throw std::runtime_error("Strings file missing 'strings.months' structure");
    }

    const auto& monthsArray = data["strings"]["months"];

    if (!monthsArray.is_array() || monthsArray.size() != 12)
    {
        throw std::runtime_error("Expected 12 months in strings file");
    }

    std::array<std::string, 12> monthStrings;

    for (size_t i = 0; i < 12; ++i)
    {
        monthStrings[i] = monthsArray[i].get<std::string>();
    }

    return monthStrings;
}


nlohmann::json GameDataLoader::loadTools()
{
    return nlohmann::json();
}

