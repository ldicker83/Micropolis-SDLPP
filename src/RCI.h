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


class RCI
{
public:
	RCI() = default;
	RCI(int residentialDemand, int commercialDemand, int industrialDemand);


	void adjustResidentialDemand(int delta);
	void adjustCommercialDemand(int delta);
	void adjustIndustrialDemand(int delta);

	void residentialDemand(int demand) { mResidentialDemand = demand; }
	int residentialDemand() const { return mResidentialDemand; }
	
	void commercialDemand(int demand) { mCommercialDemand = demand; }
	int commercialDemand() const { return mCommercialDemand; }
	
	void industrialDemand(int demand) { mIndustrialDemand = demand; }
	int industrialDemand() const { return mIndustrialDemand; }

private:
	int mResidentialDemand{ 0 };
	int mCommercialDemand{ 0 };
	int mIndustrialDemand{ 0 };
};