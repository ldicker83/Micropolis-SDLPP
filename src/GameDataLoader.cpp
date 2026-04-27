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

    assertJsonContainsKey(data, Constants::JsonKeys::Strings, Constants::ErrorMessages::StringsFileMissingStrings);
    assertJsonContainsKey(data[Constants::JsonKeys::Strings], Constants::JsonKeys::Tools, Constants::ErrorMessages::StringsFileMissingTools);

	const auto& toolsData = data[Constants::JsonKeys::Strings][Constants::JsonKeys::Tools];
	return {
		{},
		{ Tool::Type::Residential, 100, 3, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Residential) },
		{ Tool::Type::Commercial, 100, 3, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Commercial) },
		{ Tool::Type::Industrial, 100, 3, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Industrial) },
		{ Tool::Type::Fire, 500, 3, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Fire) },
		{ Tool::Type::Query, 0, 1, 0, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Query) },
		{ Tool::Type::Police, 500, 3, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Police) },
		{ Tool::Type::Wire, 5, 1, 0, true, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Wire) },
		{ Tool::Type::Bulldoze, 1, 1, 0, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Bulldoze) },
		{ Tool::Type::Rail, 20, 1, 0, true, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Rail) },
		{ Tool::Type::Road, 10, 1, 0, true, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Road) },
		{ Tool::Type::Stadium, 5000, 4, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Stadium) },
		{ Tool::Type::Park, 10, 1, 0, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Park) },
		{ Tool::Type::Seaport, 3000, 4, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Seaport) },
		{ Tool::Type::Coal, 3000, 4, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Coal) },
		{ Tool::Type::Nuclear, 5000, 4, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Nuclear) },
		{ Tool::Type::Airport, 10000, 6, 1, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Airport) },
		{ Tool::Type::Network, 100, 1, 0, false, jsonStringValue(toolsData, Constants::JsonKeys::ToolTypes::Network) }
	};
}
