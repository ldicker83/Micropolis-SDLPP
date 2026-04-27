#include "GameDataLoader.h"

#include "Tool.h"

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


    nlohmann::json parseJsonFile(const std::string& filePath)
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


    std::string jsonStringValue(const nlohmann::json& data, const std::string& key)
    {
        assertJsonContainsKey(data, key, std::format(Constants::ErrorMessages::ToolKeyNotFound, key));
        return data[key].get<std::string>();
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
	auto jsonData = parseJsonFile(Constants::FilePaths::Strings);
	return jsonData;
}


MonthStringArray GameDataLoader::loadMonthStrings()
{
    auto data = parseJsonFile(Constants::FilePaths::Strings);
	assertJsonContainsKey(data, Constants::JsonKeys::Strings, Constants::ErrorMessages::StringsFileMissingStrings);
	assertJsonContainsKey(data[Constants::JsonKeys::Strings], Constants::JsonKeys::Months, Constants::ErrorMessages::StringsFileMissingMonths);

    return loadMonthStringsFromJson(data[Constants::JsonKeys::Strings][Constants::JsonKeys::Months]);
}


std::vector<Tool> GameDataLoader::loadTools()
{
    const auto data = parseJsonFile(Constants::FilePaths::Strings);

    assertJsonContainsKey(data, "strings", Constants::ErrorMessages::StringsFileMissingStrings);
    assertJsonContainsKey(data["strings"], "tools", Constants::ErrorMessages::StringsFileMissingTools);

	const auto& toolsData = data["strings"]["tools"];
    return {
        {},
        { Tool::Type::Residential, 100, 3, 1, false, jsonStringValue(toolsData, "residential") },
        { Tool::Type::Commercial, 100, 3, 1, false, jsonStringValue(toolsData, "commercial") },
        { Tool::Type::Industrial, 100, 3, 1, false, jsonStringValue(toolsData, "industrial") },
        { Tool::Type::Fire, 500, 3, 1, false, jsonStringValue(toolsData, "fire") },
        { Tool::Type::Query, 0, 1, 0, false, jsonStringValue(toolsData, "query") },
        { Tool::Type::Police, 500, 3, 1, false, jsonStringValue(toolsData, "police") },
        { Tool::Type::Wire, 5, 1, 0, true, jsonStringValue(toolsData, "wire") },
        { Tool::Type::Bulldoze, 1, 1, 0, false, jsonStringValue(toolsData, "bulldoze") },
        { Tool::Type::Rail, 20, 1, 0, true, jsonStringValue(toolsData, "rail") },
        { Tool::Type::Road, 10, 1, 0, true, jsonStringValue(toolsData, "road") },
        { Tool::Type::Stadium, 5000, 4, 1, false, jsonStringValue(toolsData, "stadium") },
        { Tool::Type::Park, 10, 1, 0, false, jsonStringValue(toolsData, "park") },
        { Tool::Type::Seaport, 3000, 4, 1, false, jsonStringValue(toolsData, "seaport") },
        { Tool::Type::Coal, 3000, 4, 1, false, jsonStringValue(toolsData, "coal") },
        { Tool::Type::Nuclear, 5000, 4, 1, false, jsonStringValue(toolsData, "nuclear") },
        { Tool::Type::Airport, 10000, 6, 1, false, jsonStringValue(toolsData, "airport") },
        { Tool::Type::Network, 100, 1, 0, false, jsonStringValue(toolsData, "network") }
    };
}
