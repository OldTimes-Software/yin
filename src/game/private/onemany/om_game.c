// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "om_game.h"

OMPawn       omPawns[ OM_MAX_PAWNS ];
unsigned int omNumPlayers = 0;

static OMGameState omGameState;

static unsigned int       omSplashTimer = 0;
static const unsigned int omSplashTimeout = 100;

static PLGTexture *textures[ OM_MAX_TEXTURES ];

static void OM_Game_Initialize( void )
{
	// todo: init this under game.c instead!
	clientInterface = Engine_Public_GetClientInterface( ENGINE_CLIENT_INTERFACE_VERSION );

	textures[ OM_TEXTURE_BACKGROUND ] = clientInterface->renderer.LoadTexture( "guis/assets/om/bg.png", PLG_TEXTURE_FILTER_NEAREST );
	textures[ OM_TEXTURE_BOARD ] = clientInterface->renderer.LoadTexture( "guis/assets/om/board.bmp", PLG_TEXTURE_FILTER_NEAREST );
	textures[ OM_TEXTURE_PAWN_CAR ] = clientInterface->renderer.LoadTexture( "guis/assets/om/car.png", PLG_TEXTURE_FILTER_NEAREST );
	textures[ OM_TEXTURE_PAWN_DOG ] = clientInterface->renderer.LoadTexture( "guis/assets/om/dog.png", PLG_TEXTURE_FILTER_NEAREST );
	textures[ OM_TEXTURE_PAWN_MAN ] = clientInterface->renderer.LoadTexture( "guis/assets/om/man.png", PLG_TEXTURE_FILTER_NEAREST );
	textures[ OM_TEXTURE_PAWN_VAMP ] = clientInterface->renderer.LoadTexture( "guis/assets/om/vmp.png", PLG_TEXTURE_FILTER_NEAREST );

	PlParseConsoleString( "world empty" );
}

static void OM_Game_Shutdown( void )
{
}

static void OM_Game_NewGame( const char *path )
{
	omGameState = OM_GAME_STATE_SPLASH;
}

static void OM_Game_SaveGame( const char *path ) {}
static void OM_Game_RestoreGame( const char *path ) {}

static void OM_Game_Tick( void )
{
	switch ( omGameState )
	{
		case OM_GAME_STATE_SPLASH:
			break;
	}
}

static void OM_Game_DrawMenu( const PLGViewport *viewport )
{
}

const GameModeInterface *gameModeInterface = NULL;
const GameModeInterface *Game_GetModeInterface( void )
{
	static GameModeInterface gameMode;
	PL_ZERO_( gameMode );

	gameMode.Initialize = OM_Game_Initialize;
	gameMode.Shutdown = OM_Game_Shutdown;
	gameMode.NewGame = OM_Game_NewGame;
	gameMode.SaveGame = OM_Game_SaveGame;
	gameMode.RestoreGame = OM_Game_RestoreGame;
	gameMode.Tick = OM_Game_Tick;
	gameMode.DrawMenu = OM_Game_DrawMenu;

	return &gameMode;
}
