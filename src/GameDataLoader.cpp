#include "GameDataLoader.h"

#include "Constants.h"

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
	auto jsonData = loadJsonFromFile(Constants::FilePaths::Strings);
	return jsonData;
}


std::array<std::string, 12> GameDataLoader::loadMonthStrings()
{
    auto data = loadJsonFromFile(Constants::FilePaths::Strings);

    if (!data.contains(Constants::JsonKeys::Strings) || 
        !data[Constants::JsonKeys::Strings].contains(Constants::JsonKeys::Months))
    {
        throw std::runtime_error(Constants::ErrorMessages::StringsFileMissingMonths);
    }

    const auto& monthsArray = data[Constants::JsonKeys::Strings][Constants::JsonKeys::Months];

    if (!monthsArray.is_array() || monthsArray.size() != 12)
    {
        throw std::runtime_error(Constants::ErrorMessages::ExpectedTwelveMonths);
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

