#include "GameDataLoader.h"

#include "Tool.h"

#include <format>
#include <fstream>
#include <unordered_map>


namespace
{
    const std::unordered_map<std::string, Tool::Type> ToolTypeTable = {
        { Constants::JsonKeys::ToolTypes::Residential, Tool::Type::Residential },
        { Constants::JsonKeys::ToolTypes::Commercial, Tool::Type::Commercial },
        { Constants::JsonKeys::ToolTypes::Industrial, Tool::Type::Industrial },
        { Constants::JsonKeys::ToolTypes::Fire, Tool::Type::Fire },
        { Constants::JsonKeys::ToolTypes::Query, Tool::Type::Query },
        { Constants::JsonKeys::ToolTypes::Police, Tool::Type::Police },
        { Constants::JsonKeys::ToolTypes::Wire, Tool::Type::Wire },
        { Constants::JsonKeys::ToolTypes::Bulldoze, Tool::Type::Bulldoze },
        { Constants::JsonKeys::ToolTypes::Rail, Tool::Type::Rail },
        { Constants::JsonKeys::ToolTypes::Road, Tool::Type::Road },
        { Constants::JsonKeys::ToolTypes::Stadium, Tool::Type::Stadium },
        { Constants::JsonKeys::ToolTypes::Park, Tool::Type::Park },
        { Constants::JsonKeys::ToolTypes::Seaport, Tool::Type::Seaport },
        { Constants::JsonKeys::ToolTypes::Coal, Tool::Type::Coal },
        { Constants::JsonKeys::ToolTypes::Nuclear, Tool::Type::Nuclear },
        { Constants::JsonKeys::ToolTypes::Airport, Tool::Type::Airport },
        { Constants::JsonKeys::ToolTypes::Network, Tool::Type::Network }
    };

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


    int jsonIntValue(const nlohmann::json& data, const std::string& key)
    {
        assertJsonContainsKey(data, key, std::format(Constants::ErrorMessages::ToolMissingField, key));
        return data[key].get<int>();
    }


    bool jsonBoolValue(const nlohmann::json& data, const std::string& key)
    {
        assertJsonContainsKey(data, key, std::format(Constants::ErrorMessages::ToolMissingField, key));
        return data[key].get<bool>();
    }


    Tool::Type toolType(const std::string& type)
    {
        try
        {
			return ToolTypeTable.at(type);
        }
		catch (const std::out_of_range&)
        {
            throw std::runtime_error(std::format(Constants::ErrorMessages::ToolTypeUnknown, type));
		}

    }


    Tool toolFromJson(const nlohmann::json& toolData, const nlohmann::json& stringsData)
    {
        const auto typeString = jsonStringValue(toolData, Constants::JsonKeys::Type);

        return {
            toolType(typeString),
            jsonIntValue(toolData, Constants::JsonKeys::Cost),
            jsonIntValue(toolData, Constants::JsonKeys::Size),
            jsonIntValue(toolData, Constants::JsonKeys::Offset),
            jsonBoolValue(toolData, Constants::JsonKeys::Draggable),
            jsonStringValue(stringsData, typeString)
        };
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
	const auto toolsFileData = parseJsonFile(Constants::FilePaths::Tools);
	const auto stringsFileData = parseJsonFile(Constants::FilePaths::Strings);

	assertJsonContainsKey(toolsFileData, Constants::JsonKeys::Tools, Constants::ErrorMessages::ToolsFileMissingTools);
	assertJsonContainsKey(stringsFileData, Constants::JsonKeys::Strings, Constants::ErrorMessages::StringsFileMissingStrings);
	assertJsonContainsKey(stringsFileData[Constants::JsonKeys::Strings], Constants::JsonKeys::Tools, Constants::ErrorMessages::StringsFileMissingTools);

	const auto& toolsArray = toolsFileData[Constants::JsonKeys::Tools];
	const auto& toolStringsData = stringsFileData[Constants::JsonKeys::Strings][Constants::JsonKeys::Tools];

	assertJsonDataIsArray(toolsArray);

	std::vector<Tool> tools;
	tools.push_back({});  // First tool is empty/None

	for (const auto& toolData : toolsArray)
	{
		tools.push_back(toolFromJson(toolData, toolStringsData));
	}

	return tools;
}
