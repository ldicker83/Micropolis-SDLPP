// This file is part of Micropolis-SDL2PP
// Micropolis-SDL2PP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDL2PP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#include "Evaluation.h"

#include "Budget.h"
#include "CityProperties.h"
#include "Map.h"

#include "s_alloc.h"
#include "s_sim.h"

#include "w_tk.h"
#include "w_update.h"

#include <algorithm>


/* City Evaluation */
namespace
{
    int CityYes{}, CityNo{};

    bool EvalChanged{ false };

    Evaluation CurrentEvaluation;

    constexpr auto ProblemArraySize = 10;

    struct Problem
    {
        std::string name{};
        int value{};
        int votes{};
    };

    std::array<Problem, ProblemArraySize> Problems;

    int CityPop{}, deltaCityPop{};
    int CityAssessedValue;
    CityClass CityClassValue;
    int CityScore{}, DeltaCityScore{}, AverageCityScore{};
    int TrafficAverage{};

    constexpr int CityClassPopulation[] =
    {
        0, 2000, 10000, 50000, 100000, 500000
    };

    const std::string CityClassString[] =
    {
         "Village", "Town", "City", "Capital", "Metropolis", "Megalopolis"
    };

    const std::string CityLevelString[] =
    {
      "Easy", "Medium", "Hard"
    };

    const std::string ProblemTitles[10] =
    {
      "Crime",
      "Pollution",
      "Housing Costs",
      "Taxes",
      "Traffic",
      "Unemployment",
      "Fires"
    };


    int PopulationThreshold(CityClass cityClass)
    {
        return CityClassPopulation[static_cast<int>(cityClass)];
    }


    /**
     * \todo Yuck, there has to be a better way to do this.
     */
    CityClass DetermineCityClass()
    {
        auto cityClass = CityClass::Village;

        if (CityPop > PopulationThreshold(CityClass::Town))
        {
            cityClass = CityClass::Town;
        }

        if (CityPop > PopulationThreshold(CityClass::City))
        {
            cityClass = CityClass::City;
        }

        if (CityPop > PopulationThreshold(CityClass::Capital))
        {
            cityClass = CityClass::Capital;
        }

        if (CityPop > PopulationThreshold(CityClass::Metropolis))
        {
            cityClass = CityClass::Metropolis;
        }

        if (CityPop > PopulationThreshold(CityClass::Megalopolis))
        {
            cityClass = CityClass::Megalopolis;
        }

        return cityClass;
    }

};


const Evaluation& currentEvaluation()
{
    return CurrentEvaluation;
}


void currentEvaluationSeen()
{
    CurrentEvaluation.needsAttention = false;
}


int trafficAverage()
{
    return TrafficAverage;
}


int cityAssessedValue()
{
    return CityAssessedValue;
}


int cityScore()
{
    return CityScore;
}


void cityScore(const int score)
{
    CityScore = score;
}


int deltaCityScore()
{
    return DeltaCityScore;
}


CityClass cityClass()
{
    return static_cast<CityClass>(CityClassValue);
}


void cityClass(const CityClass value)
{
    CityClassValue = value;
}


int cityYes()
{
    return CityYes;
}


int cityNo()
{
    return CityNo;
}


int cityPopulation()
{
    return CityPop;
}


void cityPopulation(const int val)
{
    CityPop = val;
}


int deltaCityPopulation()
{
    return deltaCityPop;
}


void EvalInit()
{
    CityYes = 0;
    CityNo = 0;
    CityPop = 0;
    deltaCityPop = 0;
    CityAssessedValue = 0;
    CityClassValue = CityClass::Village;
    CityScore = 500;
    DeltaCityScore = 0;
    
    Problems.fill({});
}


void ChangeEval()
{
    EvalChanged = true;
}


void GetAssessedValue()
{
    int assesedValue = RoadCount * 5;
    assesedValue += RailCount * 10;
    assesedValue += PoliceStationCount * 1000;
    assesedValue += FireStationCount * 1000;
    assesedValue += HospitalCount * 400;
    assesedValue += StadiumCount * 3000;
    assesedValue += SeaPortCount * 5000;
    assesedValue += AirportCount * 10000;
    assesedValue += CoalPowerPlantCount * 3000;
    assesedValue += NuclearPowerPlantCount * 6000;
    CityAssessedValue = assesedValue * 1000;
}


void DoPopNum()
{
    int oldCityPop{ CityPop };
    CityPop = (ResidentialPopulationCount + (CommercialPopulationCount * 8) + (IndustrialPopulationCount * 8)) * 20;
    
    if (oldCityPop == 0)
    {
        oldCityPop = CityPop;
    }
    
    deltaCityPop = CityPop - oldCityPop;
    
    CityClassValue = DetermineCityClass();
}


void VoteProblems()
{
    int problemIndex{}, voteCount{}, count{};
    while ((voteCount < 100) && (count < 600))
    {
        if (randomRange(0, 300) < Problems[problemIndex].value)
        {
            ++Problems[problemIndex].votes;
            ++voteCount;
        }
        
        ++problemIndex;

        if (problemIndex >= ProblemArraySize)
        {
            problemIndex = 0;
        }
        ++count;
    }
}


int AverageTraffic()
{
    int trafficTotal{};
    int count{ 1 };
    
    for (int x{}; x < HalfWorldWidth; ++x)
    {
        for (int y{}; y < HalfWorldHeight; ++y)
        {
            if (LandValueMap.value({ x, y }))
            {
                trafficTotal += TrafficDensityMap.value({ x, y });
                ++count;
            }
        }
    }

    TrafficAverage = static_cast<int>((static_cast<float>(trafficTotal) / count) * 2.4f);
    return TrafficAverage;
}


int GetUnemployment()
{
    float ratio{ 0.0f };

    int base{ (CommercialPopulationCount + IndustrialPopulationCount) * 8 };
    if (base)
    {
        ratio = (static_cast<float>(ResidentialPopulationCount)) / base;
    }
    else
    {
        return 0;
    }

    base = static_cast<int>((ratio - 1) * 255);
    if (base > 255)
    {
        base = 255;
    }

    return base;
}


int GetFire()
{
    return std::clamp(BurningTileCount * 5, 0, 255);
}


void GetScore(const Budget& budget)
{
    int x, z;
    int OldCityScore;
    float SM, TM;

    OldCityScore = CityScore;
    x = 0;
    for (z = 0; z < ProblemArraySize; z++)
    {
        x += Problems[z].value;
    }

    x = x / 3;			/* 7 + 2 average */
    if (x > 256)
    {
        x = 256;
    }

    z = (256 - x) * 4;
    if (z > 1000)
    {
        z = 1000;
    }
    if (z < 0)
    {
        z = 0;
    }

    if (ResCap) { z = static_cast<int>(z * .85); }
    if (ComCap) { z = static_cast<int>(z * .85); }
    if (IndCap) { z = static_cast<int>(z * .85); }
    if (RoadEffect < 32) { z = z - (32 - RoadEffect); }
    if (PoliceEffect < 1000) { z = static_cast<int>(z * (.9 + (PoliceEffect / 10000.1))); }
    if (FireEffect < 1000) { z = static_cast<int>(z * (.9 + (FireEffect / 10000.1))); }

	const auto& rci = currentRCI();

    if (rci.residentialDemand() < -1000) { z = static_cast<int>(z * .85); }
    if (rci.commercialDemand() < -1000) { z = static_cast<int>(z * .85); }
    if (rci.industrialDemand() < -1000) { z = static_cast<int>(z * .85); }

    SM = 1.0;
    if ((CityPop == 0) || (deltaCityPop == 0))
    {
        SM = 1.0;
    }
    else if (deltaCityPop == CityPop)
    {
        SM = 1.0;
    }
    else if (deltaCityPop > 0)
    {
        SM = ((float)deltaCityPop / CityPop) + 1.0f;
    }
    else if (deltaCityPop < 0)
    {
        SM = .95f + ((float)deltaCityPop / (CityPop - deltaCityPop));
    }
    z = static_cast<int>(z * SM);
    z = z - GetFire();		/* dec score for fires */
    z = z - (budget.TaxRate());

    TM = static_cast<float>(UnpoweredZoneCount + PoweredZoneCount);	/* dec score for unpowered zones */
    if (TM) { SM = PoweredZoneCount / TM; }
    else { SM = 1.0; }
    z = static_cast<int>(z * SM);

    if (z > 1000) { z = 1000; }
    if (z < 0) { z = 0; }

    CityScore = (CityScore + z) / 2;

    DeltaCityScore = CityScore - OldCityScore;
}


void DoVotes()
{
    CityYes = 0;
    CityNo = 0;
    for (int z{}; z < 100; z++)
    {
        if (randomRange(0, 1000) < CityScore)
        {
            CityYes++;
        }
        else
        {
            CityNo++;
        }
    }
}


void DoProblems(const Budget& budget)
{
    Problems =
    {
        Problem{ ProblemTitles[0], CrimeAverage },
        Problem{ ProblemTitles[1], PolluteAverage },
        Problem{ ProblemTitles[2], static_cast<int>(LVAverage * 0.7f) },
        Problem{ ProblemTitles[3], budget.TaxRate() * 10 },
        Problem{ ProblemTitles[4], AverageTraffic() },
        Problem{ ProblemTitles[5], GetUnemployment() },
        Problem{ ProblemTitles[6], GetFire() }
    };

    VoteProblems();

    std::sort(Problems.begin(), Problems.end(), [](const Problem& a, const Problem& b) { return a.votes > b.votes; });
}


void CityEvaluation(const Budget& budget)
{
    if (PopulationTotal)
    {
        GetAssessedValue();
        DoPopNum();
        DoProblems(budget);
        GetScore(budget);
        DoVotes();
        ChangeEval();
    }
    else
    {
        EvalInit();
        ChangeEval();
    }
}


void buildEvaluationReport(const CityProperties& properties)
{
    CurrentEvaluation =
    {
        std::to_string(deltaCityScore()),
        std::to_string(cityScore()),
        std::array<std::string, 4>
        {
            Problems[0].name,
            Problems[1].name,
            Problems[2].name,
            Problems[3].name
        },
        std::array<std::string, 4>
        {
            std::to_string(Problems[0].votes) + "%",
            std::to_string(Problems[1].votes) + "%",
            std::to_string(Problems[2].votes) + "%",
            std::to_string(Problems[3].votes) + "%"
        },
        std::to_string(cityPopulation()),
        std::to_string(deltaCityPopulation()),
        numberToDollarDecimal(cityAssessedValue()),
        CityClassString[static_cast<int>(cityClass())],
        CityLevelString[properties.GameLevel()],
        std::to_string(cityYes()) + "%",
        std::to_string(cityNo()) + "%",
        std::to_string(currentYear())
    };
}


void refreshCityEvaluation(const CityProperties& properties)
{
    if (EvalChanged)
    {
        buildEvaluationReport(properties);
        EvalChanged = false;
    }
}
