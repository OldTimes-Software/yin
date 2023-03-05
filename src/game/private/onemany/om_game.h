// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "../game_private.h"

#define OM_SCREEN_WIDTH  320
#define OM_SCREEN_HEIGHT 240

typedef enum OMTexture
{
	OM_TEXTURE_SPLASH,
	OM_TEXTURE_BACKGROUND,
	OM_TEXTURE_BOARD,

	OM_TEXTURE_PAWN_MAN,
	OM_TEXTURE_PAWN_CAR,
	OM_TEXTURE_PAWN_DOG,
	OM_TEXTURE_PAWN_VAMP,

	OM_MAX_TEXTURES
} OMTexture;

typedef enum OMGameState
{
	OM_GAME_STATE_SPLASH,
	OM_GAME_STATE_START,
	OM_GAME_STATE_PLAY,
	OM_GAME_STATE_PAUSED,

	OM_MAX_GAME_STATES
} OMGameState;

typedef enum OMCurrencyType
{
	OM_CURRENCY_BOTTLECAPS,
	OM_CURRENCY_BOLTS,
	OM_CURRENCY_CREDITS,
	OM_CURRENCY_DRACHMA,
	OM_CURRENCY_DIRT,
	OM_CURRENCY_RINGS,
	OM_CURRENCY_MOOLAH,

	OM_MAX_CURRENCYS
} OMCurrencyType;

typedef enum OMPawnType
{
	OM_PAWN_MAN,
	OM_PAWN_CAR,
	OM_PAWN_DOG,
	OM_PAWN_VAMP,

	OM_MAX_PAWNS
} OMPawnType;

typedef struct OMPawn
{
	uint8_t      space;                       // current space we're occupying
	unsigned int currency[ OM_MAX_CURRENCYS ];// amount of money (per-type)
} OMPawn;

extern OMPawn       omPawns[ OM_MAX_PAWNS ];
extern unsigned int omNumPlayers;
