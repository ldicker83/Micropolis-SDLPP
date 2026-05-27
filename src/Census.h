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


struct Census
{
    int PoweredZoneCount{ 0 };
    int UnpoweredZoneCount{ 0 };

    int BurningTileCount{ 0 };
    
    int RoadCount{ 0 };
    int RailCount{ 0 };
    
    int ResidentialPopulationCount{ 0 };
    int CommercialPopulationCount{ 0 };
    int IndustrialPopulationCount{ 0 };
    
    int ResidentialZoneCount{ 0 };
    int CommercialZoneCount{ 0 };
    int IndustrialZoneCount{ 0 };
    
    int HospitalCount{ 0 };
    int ChurchCount{ 0 };
    
    int PoliceStationCount{ 0 };
    int FireStationCount{ 0 };
    
    int StadiumCount{ 0 };
    
    int CoalPowerPlantCount{ 0 };
    int NuclearPowerPlantCount{ 0 };
    
    int SeaPortCount{ 0 };
    int AirportCount{ 0 };


    void clear()
    {
		*this = {};
    }
};
