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
#pragma once

#include <array>
#include <string>

class Budget;
class CityProperties;


struct Evaluation
{
    std::string changed{};
    std::string score{};

    std::array<std::string, 4> problemString;
    std::array<std::string, 4> problemVote;

    std::string pop{};
    std::string delta{};
    std::string assessed_dollars{};

    std::string cityclass{};
    std::string citylevel{};

    std::string goodyes{};
    std::string goodno{};

    std::string title{};

    bool needsAttention{ true };
};


enum class CityClass
{
	Village,
	Town,
	City,
	Capital,
	Metropolis,
	Megalopolis
};


const Evaluation& currentEvaluation();

void currentEvaluationSeen();

int cityAssessedValue();

CityClass cityClass();
void cityClass(const CityClass value);

void CityEvaluation(const Budget& budget);

int cityPopulation();
void cityPopulation(const int val);

int cityScore();
void cityScore(const int score);

int cityNo();
int cityYes();

int deltaCityPopulation();
int deltaCityScore();

void EvalInit();

int trafficAverage();

void ChangeEval();
void refreshCityEvaluation(const CityProperties& properties);
