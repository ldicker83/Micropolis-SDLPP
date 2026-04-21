// This file is part of Micropolis-SDLPP
// Micropolis-SDLPP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDLPP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.

#include "Budget.h"
#include "CityProperties.h"
#include "Map.h"

#include "s_alloc.h"
#include "FileIo.h"
#include "s_sim.h"

#include "w_sound.h"
#include "w_tk.h"
#include "w_update.h"
#include "Util.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <string>


namespace
{
    struct ScenarioProperties
    {
        const std::string FileName{};
        const std::string CityName{};
        const int Time{};
        const int StartingFunds{};
        const int Id{};
    };

    std::map<Scenario, ScenarioProperties> ScenarioPropertiesTable
    {
        { Scenario::Dullsville, { "snro.111", "Dullsville", ((1900 - 1900) * 48) + 2, 5000, 1 } },
        { Scenario::SanFransisco, { "snro.222", "San Francisco", ((1906 - 1900) * 48) + 2, 20000, 2 } },
        { Scenario::Hamburg, { "snro.333", "Hamburg", ((1944 - 1900) * 48) + 2, 20000, 3 } },
        { Scenario::Bern, { "snro.444", "Bern", ((1965 - 1900) * 48) + 2, 20000, 4 } },
        { Scenario::Tokyo, { "snro.555", "Tokyo", ((1957 - 1900) * 48) + 2, 20000, 5 } },
        { Scenario::Detroit, { "snro.666", "Detroit", ((1972 - 1900) * 48) + 2, 20000, 6 } },
        { Scenario::Boston, { "snro.777", "Boston", ((2010 - 1900) * 48) + 2, 20000, 7 } },
        { Scenario::Rio, { "snro.888", "Rio de Janeiro", ((2047 - 1900) * 48) + 2, 20000, 8 } }
    };


    void copyBufIntoArray(const int(&buf)[HistoryLength], GraphHistory& graph)
    {
        for (size_t i = 0; i < ResidentialPopulationHistory.size(); ++i)
        {
            graph[i] = buf[i];
        }
    }


    bool loadFile(const std::string filename)
    {
        std::ifstream infile(filename, std::ofstream::binary);
        if (infile.fail())
        {
            infile.close();
            return false;
        }

        int buff[HistoryLength]{};

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, ResidentialPopulationHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, CommercialPopulationHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, IndustrialPopulationHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, CrimeHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, PollutionHistory);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, MoneyHis);

        infile.read(reinterpret_cast<char*>(&buff[0]), sizeof(GraphHistory));
        copyBufIntoArray(buff, MiscHistory);

        const auto mapData = getMapData();
        infile.read(const_cast<char*>(mapData.data), mapData.size);

        infile.close();

        return true;
    }


    bool loadGame(const std::string& filename, CityProperties& properties, Budget& budget)
    {
        if (!loadFile(filename))
        {
            return false;
        }

        CityTime = std::clamp(MiscHistory[8], 0, std::numeric_limits<int>::max());
        budget.CurrentFunds(MiscHistory[50]);
        budget.PreviousFunds(MiscHistory[51]);
        gameplayOptions().autoBulldoze = MiscHistory[52];
        gameplayOptions().autoBudget = MiscHistory[53];
        gameplayOptions().autoGoto = MiscHistory[54];

        userSoundOn(MiscHistory[55]);
        budget.TaxRate(std::clamp(MiscHistory[56], 0, 20));
        simSpeed(static_cast<SimulationSpeed>(MiscHistory[57]));

        budget.PolicePercent(static_cast<float>(MiscHistory[58] / 100.0f));
        budget.FirePercent(static_cast<float>(MiscHistory[60] / 100.0f));
        budget.RoadPercent(static_cast<float>(MiscHistory[62] / 100.0f));

        initWillStuff();
        ScenarioID = 0;
        DoSimInit(properties, budget);

        return true;
    }


    bool saveGame(const std::string& filename, const CityProperties&, const Budget& budget)
    {
        std::ofstream outfile(filename, std::ofstream::binary);
        if (outfile.fail())
        {
            outfile.close();
            return false;
        }

        MiscHistory[8] = CityTime;
        MiscHistory[50] = budget.CurrentFunds();
        MiscHistory[51] = budget.PreviousFunds();

        MiscHistory[52] = gameplayOptions().autoBulldoze;
        MiscHistory[53] = gameplayOptions().autoBudget;
        MiscHistory[54] = gameplayOptions().autoGoto;
        MiscHistory[55] = userSoundOn();
        MiscHistory[57] = static_cast<int>(simSpeed());
        MiscHistory[56] = budget.TaxRate();

        MiscHistory[58] = static_cast<int>(budget.PolicePercent() * 100.0f);
        MiscHistory[60] = static_cast<int>(budget.FirePercent() * 100.0f);
        MiscHistory[62] = static_cast<int>(budget.RoadPercent() * 100.0f);

        outfile.write(reinterpret_cast<char*>(ResidentialPopulationHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(CommercialPopulationHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(IndustrialPopulationHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(CrimeHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(PollutionHistory.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(MoneyHis.data()), sizeof(GraphHistory));
        outfile.write(reinterpret_cast<char*>(MiscHistory.data()), sizeof(GraphHistory));

        const auto mapData = getMapData();
        outfile.write(mapData.data, mapData.size);

        outfile.close();
        return true;
    }


    std::string extractFilenameWithoutExtension(const std::string& filepath)
    {
        return std::filesystem::path(filepath).stem().string();
    }
}


bool LoadCity(const std::string& filename, CityProperties& properties, Budget& budget)
{
    if(!loadGame(filename, properties, budget))
    {
        std::cout << "Unable to load a city from the file named '" << filename << "'" << std::endl;
        return false;
    }

    properties.CityName(extractFilenameWithoutExtension(filename));

    return true;
}


void SaveCity(const std::string& filename, const CityProperties& properties, const Budget& budget)
{
    if (saveGame(filename, properties, budget))
    {
        std::cout << "City saved as '" << filename << "'" << std::endl;
    }
    else
    {
        std::cout << "Unable to save the city to the file named '" << filename << "'" << std::endl;
    }
}


void LoadScenario(Scenario scenario, CityProperties& properties, Budget& budget)
{
    properties.GameLevel(0);

    const auto& scenarioProperties = ScenarioPropertiesTable.at(scenario);

    properties.CityName(scenarioProperties.CityName);
    budget.CurrentFunds(scenarioProperties.StartingFunds);
    CityTime = scenarioProperties.Time;
    ScenarioID = scenarioProperties.Id;

    ResetMap();
    loadFile("scenarios/" + scenarioProperties.FileName);

    simSpeed(SimulationSpeed::Normal);

    initWillStuff();
    updateFunds(budget);
    InitSimLoad = 1;
    DoInitialEval = 0;
    DoSimInit(properties, budget);
}
