// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "../game_private.h"

#define FW_MAX_TEAMS        4
#define FW_MAX_TEAM_MEMBERS 64
#define FW_MAX_TEAM_PLAYERS FW_MAX_TEAM_MEMBERS

#define FW_MAX_TEAM_NAME 16

typedef struct FWTeam
{
	char name[ FW_MAX_TEAM_NAME ];
} FWTeam;

#include "fw_simulation.h"

typedef struct FWGameState
{
	FWSimState simState;

	PLLinkedList *buildings;
} FWGameState;
extern FWGameState fwGameState;
