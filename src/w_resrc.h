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

#include <string>

enum class NotificationId
{
    None,
    ResidentialNeeded,
    CommercialNeeded,
    IndustrialNeeded,
    RoadsNeeded,
    RailNeeded,
    PowerNeeded,
    StadiumNeeded,
    SeaportNeeded,
    AirportNeeded,
    PollutionHigh,
    CrimeHigh,
    TrafficJamsReported,
    FireDepartmentNeeded,
    PoliceDepartmentNeeded,
    BlackoutsReported,
    TaxesHigh,
    RoadsDeteriorating,
    FireDefunded,
    PoliceDefunded,
    FireReported,
    MonsterReported,
    TornadoReported,
    EarthquakeReported,
    PlaneCrashed,
    ShipWrecked,
    TrainCrashed,
    HelicopterCrashed,
    UnemploymentHigh,
    CoffersEmpty,
    FirebombingReported,
    ParksNeeded,
    ExplosionReported,
    InsufficientFunds,
    MustBulldoze,
    ReachedTown,
    ReachedCity,
    ReachedCapital,
    ReachedMetropolis,
    ReachedMegalopolis,
    BrownoutsReported,
    HeavyTrafficReported,
    FloodingReported,
    NuclearMeltdownReported,
    RiotsReported,
    EndOfDemo,
    NoSoundServer,
    NoMultiplayerLicense,
    NewCityStarted,
    CityRestored
};

enum class ZoneStatsId
{
    DensityLow,
    DensityMedium,
    DensityHigh,
    DensityVeryHigh,
    LandValueSlum,
    LandValueLowerClass,
    LandValueMiddleClass,
    LandValueHighClass,
    CrimeSafe,
    CrimeLight,
    CrimeModerate,
    CrimeDangerous,
    PollutionNone,
    PollutionModerate,
    PollutionHeavy,
    PollutionVeryHeavy,
    GrowthDeclining,
    GrowthStable,
    GrowthSlow,
    GrowthFast
};


enum class QueryStatsId
{
    Clear,
    Water,
    Trees,
    Rubble,
    Flood,
    RadioactiveWaste,
    Fire,
    Road,
    Power,
    Rail,
    Residential,
    Commercial,
    Industrial,
    Seaport,
    Airport,
    CoalPower,
    FireDepartment,
    PoliceDepartment,
    Stadium,
    NuclearPower,
    DrawBridge,
    RadarDish,
    Fountain,
    Industrial2,
    Steelers38Bears3,
    DrawBridge2,
    Ur238,
    Padding1,
    Padding2
};


struct ZoneStats
{
    std::string title;
    std::string density;
    std::string landValue;
    std::string crime;
    std::string pollution;
    std::string populationGrowth;
};


const std::string& NotificationString(NotificationId);
const std::string& ZoneStatsString(ZoneStatsId id);
const std::string& QueryStatsString(QueryStatsId id);
