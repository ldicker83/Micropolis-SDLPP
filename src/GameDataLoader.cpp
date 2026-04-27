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


    std::string getToolNameFromJson(const nlohmann::json& data, const std::string& key)
    {
		assertJsonContainsKey(data, "strings", Constants::ErrorMessages::StringsFileMissingStrings);
		assertJsonContainsKey(data["strings"], "tools", Constants::ErrorMessages::StringsFileMissingTools);

        const auto& tools = data["strings"]["tools"];

        if (!tools.contains(key))
        {
            throw std::runtime_error("Tool key '" + key + "' not found in strings.tools");
        }

        return tools[key].get<std::string>();
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

    return {
        {},
        { Tool::Type::Residential, 100, 3, 1, false, getToolNameFromJson(data, "residential") },
        { Tool::Type::Commercial, 100, 3, 1, false, getToolNameFromJson(data, "commercial") },
        { Tool::Type::Industrial, 100, 3, 1, false, getToolNameFromJson(data, "industrial") },
        { Tool::Type::Fire, 500, 3, 1, false, getToolNameFromJson(data, "fire") },
        { Tool::Type::Query, 0, 1, 0, false, getToolNameFromJson(data, "query") },
        { Tool::Type::Police, 500, 3, 1, false, getToolNameFromJson(data, "police") },
        { Tool::Type::Wire, 5, 1, 0, true, getToolNameFromJson(data, "wire") },
        { Tool::Type::Bulldoze, 1, 1, 0, false, getToolNameFromJson(data, "bulldoze") },
        { Tool::Type::Rail, 20, 1, 0, true, getToolNameFromJson(data, "rail") },
        { Tool::Type::Road, 10, 1, 0, true, getToolNameFromJson(data, "road") },
        { Tool::Type::Stadium, 5000, 4, 1, false, getToolNameFromJson(data, "stadium") },
        { Tool::Type::Park, 10, 1, 0, false, getToolNameFromJson(data, "park") },
        { Tool::Type::Seaport, 3000, 4, 1, false, getToolNameFromJson(data, "seaport") },
        { Tool::Type::Coal, 3000, 4, 1, false, getToolNameFromJson(data, "coal") },
        { Tool::Type::Nuclear, 5000, 4, 1, false, getToolNameFromJson(data, "nuclear") },
        { Tool::Type::Airport, 10000, 6, 1, false, getToolNameFromJson(data, "airport") },
        { Tool::Type::Network, 100, 1, 0, false, getToolNameFromJson(data, "network") }
    };
}
