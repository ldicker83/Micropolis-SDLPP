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

#include "w_resrc.h"

#include "Math/Point.h"

class Budget;
class InterfaceManager;

#include <memory>

void shareInterfaceManager(std::weak_ptr<InterfaceManager> manager);

void SendMes(NotificationId);
void SendMessages(const Budget&);
void ClearMes();
void SendMesAt(NotificationId, int x, int y);
void doMessage();

NotificationId MessageId();
void MessageId(NotificationId);

void MessageLocation(Point<int> location);
const Point<int>& MessageLocation();
void AutoGotoMessageLocation(bool autogo);
bool AutoGotoMessageLocation();
const std::string& LastMessage();
void MessageDisplayTime(int time);
int MessageDisplayTime();
