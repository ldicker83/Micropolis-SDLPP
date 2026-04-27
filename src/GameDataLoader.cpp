#include "GameDataLoader.h"

#include "Tool.h"

#include <format>
#include <fstream>
#include <unordered_map>


namespace
{
    const std::unordered_map<std::string, Tool::Type> ToolTypeTable = {
        { Constants::Keys::ToolTypes::Residential, Tool::Type::Residential },
        { Constants::Keys::ToolTypes::Commercial, Tool::Type::Commercial },
        { Constants::Keys::ToolTypes::Industrial, Tool::Type::Industrial },
        { Constants::Keys::ToolTypes::Fire, Tool::Type::Fire },
        { Constants::Keys::ToolTypes::Query, Tool::Type::Query },
        { Constants::Keys::ToolTypes::Police, Tool::Type::Police },
        { Constants::Keys::ToolTypes::Wire, Tool::Type::Wire },
        { Constants::Keys::ToolTypes::Bulldoze, Tool::Type::Bulldoze },
        { Constants::Keys::ToolTypes::Rail, Tool::Type::Rail },
        { Constants::Keys::ToolTypes::Road, Tool::Type::Road },
        { Constants::Keys::ToolTypes::Stadium, Tool::Type::Stadium },
        { Constants::Keys::ToolTypes::Park, Tool::Type::Park },
        { Constants::Keys::ToolTypes::Seaport, Tool::Type::Seaport },
        { Constants::Keys::ToolTypes::Coal, Tool::Type::Coal },
        { Constants::Keys::ToolTypes::Nuclear, Tool::Type::Nuclear },
        { Constants::Keys::ToolTypes::Airport, Tool::Type::Airport }
    };

    auto loadJsonFile(const std::string& filePath)
    {
        std::ifstream file(filePath);

        if (!file.is_open())
        {
            throw std::runtime_error(std::format(Constants::Errors::CouldNotOpenFile, filePath));
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
            throw std::runtime_error(std::format(Constants::Errors::JsonParseError, filePath, e.what()));
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
            throw std::runtime_error(Constants::Errors::JsonDataNotArray);
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


	void validateStringsSection(const nlohmann::json& stringsData, const std::string& subsection, const std::string& error)
    {
		assertJsonContainsKey(stringsData, Constants::Keys::Strings, Constants::Errors::StringsFileMissingStrings);
        assertJsonContainsKey(stringsData[Constants::Keys::Strings], subsection, error);
    }


    std::string jsonStringValue(const nlohmann::json& data, const std::string& key, const std::string& error)
    {
        assertJsonContainsKey(data, key, error);
        return data[key].get<std::string>();
    }


    int jsonIntValue(const nlohmann::json& data, const std::string& key, const std::string& error)
    {
        assertJsonContainsKey(data, key, error);
        return data[key].get<int>();
    }


    bool jsonBoolValue(const nlohmann::json& data, const std::string& key, const std::string& error)
    {
        assertJsonContainsKey(data, key, error);
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
            throw std::runtime_error(std::format(Constants::Errors::ToolTypeUnknown, type));
		}

    }


    Tool toolFromJson(const nlohmann::json& toolData, const nlohmann::json& stringsData)
    {
        const auto typeString = jsonStringValue(toolData, Constants::Keys::Type, std::format(Constants::Errors::ToolMissingField, Constants::Keys::Type));

        return {
            toolType(typeString),
            jsonIntValue(toolData, Constants::Keys::Cost, std::format(Constants::Errors::ToolMissingField, Constants::Keys::Cost)),
            jsonIntValue(toolData, Constants::Keys::Size, std::format(Constants::Errors::ToolMissingField, Constants::Keys::Size)),
            jsonIntValue(toolData, Constants::Keys::Offset, std::format(Constants::Errors::ToolMissingField, Constants::Keys::Offset)),
            jsonBoolValue(toolData, Constants::Keys::Draggable, std::format(Constants::Errors::ToolMissingField, Constants::Keys::Draggable)),
            jsonStringValue(stringsData, typeString, std::format(Constants::Errors::ToolKeyNotFound, typeString))
        };
    }


    MonthStringArray loadMonthStringsFromJson(const nlohmann::json& monthsArray)
    {
        assertJsonArraySize(monthsArray, Constants::MonthCount, Constants::Errors::ExpectedTwelveMonths);

        MonthStringArray monthStrings;
        for (size_t i = 0; i < Constants::MonthCount; ++i)
        {
            monthStrings[i] = monthsArray[i].get<std::string>();
        }

        return monthStrings;
    }


	void populateToolVector(std::vector<Tool>& tools, const nlohmann::json& toolsData, const nlohmann::json& toolStrings)
    {
        const auto& toolsArray = toolsData[Constants::Keys::Tools];
        assertJsonDataIsArray(toolsArray);

        for (const auto& toolData : toolsArray)
        {
            tools.push_back(toolFromJson(toolData, toolStrings));
        }
    }


    std::vector<Tool> loadToolsFromJson(const nlohmann::json& toolsData, const nlohmann::json& stringsData)
    {
        std::vector<Tool> tools;
        tools.push_back({}); // First tool is empty/None

        populateToolVector(tools, toolsData, stringsData[Constants::Keys::Strings][Constants::Keys::Tools]);
     
        return tools;
    }
}


nlohmann::json GameDataLoader::loadStrings()
{
	auto jsonData = parseJsonFile(Constants::Files::Strings);
	return jsonData;
}


MonthStringArray GameDataLoader::loadMonthStrings()
{
    auto data = parseJsonFile(Constants::Files::Strings);
	validateStringsSection(data, Constants::Keys::Months, Constants::Errors::StringsFileMissingMonths);

    return loadMonthStringsFromJson(data[Constants::Keys::Strings][Constants::Keys::Months]);
}


std::vector<Tool> GameDataLoader::loadTools()
{
	const auto stringsData = parseJsonFile(Constants::Files::Strings);
	validateStringsSection(stringsData, Constants::Keys::Tools, Constants::Errors::StringsFileMissingTools);

    const auto toolsData = parseJsonFile(Constants::Files::Tools);
    assertJsonContainsKey(toolsData, Constants::Keys::Tools, Constants::Errors::ToolsFileMissingTools);
    
    return loadToolsFromJson(toolsData, stringsData);
}
