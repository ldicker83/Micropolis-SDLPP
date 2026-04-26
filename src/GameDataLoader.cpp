#include "GameDataLoader.h"

#include <format>
#include <fstream>


namespace
{
    auto loadJsonFile(const std::string& filePath)
    {
        std::ifstream file(filePath);

        if (!file.is_open())
        {
            throw std::runtime_error(std::format(Constants::ErrorMessages::CouldNotOpenFile, filePath));
        }

        return file;
    }


    nlohmann::json loadJsonFromFile(const std::string& filePath)
    {
        auto file = loadJsonFile(filePath);

        try
        {
            return nlohmann::json::parse(file);
        }
        catch (const nlohmann::json::parse_error& e)
        {
            throw std::runtime_error(std::format(Constants::ErrorMessages::JsonParseError, filePath, e.what()));
        }
    }


    void assertJsonContainsKey(const nlohmann::json& jsonData, const std::string& key, const std::string& errorMessage)
    {
        if (!jsonData.contains(key))
        {
            throw std::runtime_error(errorMessage);
        }
    }


    void assertJsonDataIsArray(const nlohmann::json& jsonData)
    {
        if (!jsonData.is_array())
        {
            throw std::runtime_error(Constants::ErrorMessages::JsonDataNotArray);
        }
    }


    void assertJsonArraySize(const nlohmann::json& jsonData, size_t expectedSize, const std::string& errorMessage)
    {
        assertJsonDataIsArray(jsonData);

        if (jsonData.size() != expectedSize)
        {
            throw std::runtime_error(errorMessage);
        }
	}


    MonthStringArray loadMonthStringsFromJson(const nlohmann::json& monthsArray)
    {
		assertJsonArraySize(monthsArray, Constants::MonthCount, Constants::ErrorMessages::ExpectedTwelveMonths);

        MonthStringArray monthStrings;
        for (size_t i = 0; i < Constants::MonthCount; ++i)
        {
            monthStrings[i] = monthsArray[i].get<std::string>();
        }

        return monthStrings;
    }
}


nlohmann::json GameDataLoader::loadStrings()
{
	auto jsonData = loadJsonFromFile(Constants::FilePaths::Strings);
	return jsonData;
}


MonthStringArray GameDataLoader::loadMonthStrings()
{
    auto data = loadJsonFromFile(Constants::FilePaths::Strings);
	assertJsonContainsKey(data, Constants::JsonKeys::Strings, Constants::ErrorMessages::StringsFileMissingStrings);
	assertJsonContainsKey(data[Constants::JsonKeys::Strings], Constants::JsonKeys::Months, Constants::ErrorMessages::StringsFileMissingMonths);

    return loadMonthStringsFromJson(data[Constants::JsonKeys::Strings][Constants::JsonKeys::Months]);
}


nlohmann::json GameDataLoader::loadTools()
{
    return nlohmann::json();
}
