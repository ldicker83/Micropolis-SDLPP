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
#include "RCI.h"

#include <algorithm>


namespace
{
	static constexpr int ResidentialDemandMin = -2000;
	static constexpr int ResidentialDemandMax = 2000;
	static constexpr int CommercialDemandMin = -1500;
	static constexpr int CommercialDemandMax = 1500;
	static constexpr int IndustrialDemandMin = -1500;
	static constexpr int IndustrialDemandMax = 1500;
}


RCI::RCI(int residentialDemand, int commercialDemand, int industrialDemand) :
	mResidentialDemand{ residentialDemand },
	mCommercialDemand{ commercialDemand },
	mIndustrialDemand{ industrialDemand }
{}


void RCI::adjustResidentialDemand(int delta)
{
	if(delta == 0)
	{
		return;
	}

	mResidentialDemand = std::clamp(mResidentialDemand + delta, ResidentialDemandMin, ResidentialDemandMax);
}


void RCI::adjustCommercialDemand(int delta)
{
	if (delta == 0)
	{
		return;
	}

	mCommercialDemand = std::clamp(mCommercialDemand + delta, CommercialDemandMin, CommercialDemandMax);
}


void RCI::adjustIndustrialDemand(int delta)
{
	if (delta == 0)
	{
		return;
	}

	mIndustrialDemand = std::clamp(mIndustrialDemand + delta, IndustrialDemandMin, IndustrialDemandMax);
}
