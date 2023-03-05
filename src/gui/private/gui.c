// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_console.h>

#include "gui_private.h"
#include "node/public/node.h"

/****************************************
 * GUI
 ****************************************/

GUIState guiState;

static PLLinkedList *cachedTextures;
typedef struct GUICachedTexture
{
	unsigned int hash;
	PLGTexture  *texture;
} GUICachedTexture;
PLGTexture *GUI_CacheTexture( const char *path )
{
	unsigned int      hash = PlGenerateHashSDBM( path );
	PLLinkedListNode *node = PlGetFirstNode( cachedTextures );
	while ( node != NULL )
	{
		GUICachedTexture *cachedTexture = PlGetLinkedListNodeUserData( node );
		if ( cachedTexture->hash == hash )
			return cachedTexture->texture;

		node = PlGetNextLinkedListNode( node );
	}

	PLGTexture *texture = PlgLoadTextureFromImage( path, PLG_TEXTURE_FILTER_LINEAR );
	if ( texture == NULL )
		return NULL;

	GUICachedTexture *cachedTexture = PL_NEW( GUICachedTexture );
	cachedTexture->texture = texture;
	cachedTexture->hash = hash;
	PlInsertLinkedListNode( cachedTextures, cachedTexture );
	return cachedTexture->texture;
}

#define MAX_STYLE_SHEETS 16
GUIStyleSheet        styleSheets[ MAX_STYLE_SHEETS ];
const GUIStyleSheet *activeSheet = NULL;
static unsigned int  numStyleSheets = 0;

#define GUI_STYLESHEET_VERSION 1

static GUIStyleSheet *ParseStyleSheet( NLNode *root )
{
	GUIStyleSheet *guiStyleSheet = &styleSheets[ numStyleSheets ];
	PL_ZERO( guiStyleSheet, sizeof( GUIStyleSheet ) );

	int version = NL_GetI32ByName( root, "version", -1 );
	if ( version < GUI_STYLESHEET_VERSION )
	{
		GUI_Warning( "Unexpected version in stylesheet, expected %d but found %d!\n", GUI_STYLESHEET_VERSION, version );
		return NULL;
	}

	NLNode *c;
	c = NL_GetChildByName( root, "colours" );
	if ( c != NULL )
	{
		NLNode *i;
		if ( ( i = NL_GetChildByName( c, PL_STRINGIFY( GUI_COLOUR_INSET_BACKGROUND ) ) ) != NULL )
			NL_GetF32Array( i, ( float * ) &guiStyleSheet->colours[ GUI_COLOUR_INSET_BACKGROUND ], 4 );
		if ( ( i = NL_GetChildByName( c, PL_STRINGIFY( GUI_COLOUR_OUTSET_BACKGROUND ) ) ) != NULL )
			NL_GetF32Array( i, ( float * ) &guiStyleSheet->colours[ GUI_COLOUR_OUTSET_BACKGROUND ], 4 );
		if ( ( i = NL_GetChildByName( c, PL_STRINGIFY( GUI_COLOUR_INSET_BORDER_TOP ) ) ) != NULL )
			NL_GetF32Array( i, ( float * ) &guiStyleSheet->colours[ GUI_COLOUR_INSET_BORDER_TOP ], 4 );
		if ( ( i = NL_GetChildByName( c, PL_STRINGIFY( GUI_COLOUR_INSET_BORDER_BOTTOM ) ) ) != NULL )
			NL_GetF32Array( i, ( float * ) &guiStyleSheet->colours[ GUI_COLOUR_INSET_BORDER_BOTTOM ], 4 );
		if ( ( i = NL_GetChildByName( c, PL_STRINGIFY( GUI_COLOUR_OUTSET_BORDER_TOP ) ) ) != NULL )
			NL_GetF32Array( i, ( float * ) &guiStyleSheet->colours[ GUI_COLOUR_OUTSET_BORDER_TOP ], 4 );
		if ( ( i = NL_GetChildByName( c, PL_STRINGIFY( GUI_COLOUR_OUTSET_BORDER_BOTTOM ) ) ) != NULL )
			NL_GetF32Array( i, ( float * ) &guiStyleSheet->colours[ GUI_COLOUR_OUTSET_BORDER_BOTTOM ], 4 );
	}

	c = NL_GetChildByName( root, "borders" );
	if ( c != NULL )
	{
		int style = NL_GetI32ByName( c, "style", -1 );
		if ( !( style < 0 || style >= GUI_MAX_BORDER_STYLES ) )
			guiStyleSheet->borderStyle = style;
		else
			GUI_Warning( "No border style specified, using default.\n" );

		NLNode *i;
		if ( ( i = NL_GetChildByName( c, "padding" ) ) != NULL )
			NL_GetI32Array( i, guiStyleSheet->borderPadding, GUI_MAX_BORDER_ELEMENTS );
	}

	return guiStyleSheet;
}

const GUIStyleSheet *GUI_CacheStyleSheet( const char *path )
{
	NLNode *root = NL_LoadFile( path, "guiStyle" );
	if ( root == NULL )
	{
		GUI_Warning( "Failed to load node file: %s\n", NL_GetErrorMessage() );
		return NULL;
	}

	return ParseStyleSheet( root );
}

void GUI_SetStyleSheet( const GUIStyleSheet *styleSheet )
{
	activeSheet = styleSheet;
}

const GUIStyleSheet *GUI_GetActiveStyleSheet( void )
{
	return activeSheet;
}

int guiLogLevels[ GUI_MAX_LOG_LEVELS ];

/**
 * Initialize the GUI sub-system.
 */
void GUI_Initialize( void )
{
	PL_ZERO_( guiState );

	guiLogLevels[ GUI_LOGLEVEL_DEFAULT ] = PlAddLogLevel( "gui", PL_COLOUR_LIGHT_CORAL, true );
	guiLogLevels[ GUI_LOGLEVEL_WARNING ] = PlAddLogLevel( "gui/warning", PL_COLOUR_YELLOW, true );
	guiLogLevels[ GUI_LOGLEVEL_ERROR ] = PlAddLogLevel( "gui/error", PL_COLOUR_DARK_RED, true );
	guiLogLevels[ GUI_LOGLEVEL_DEBUG ] = PlAddLogLevel( "gui/debug", PL_COLOUR_CRIMSON,
#ifndef NDEBUG
	                                                    true
#else
	                                                    false
#endif
	);

	GUI_Draw_Initialize();
	GUI_BitmapFont_Initialize();

	GUI_Print( "GUI initialized!\n" );
}

void GUI_Shutdown( void )
{
	GUI_Draw_Shutdown();

	for ( unsigned int i = 0; i < GUI_MAX_LOG_LEVELS; ++i )
		PlRemoveLogLevel( guiLogLevels[ i ] );
}

void GUI_Tick( GUIPanel *root )
{
	GUI_Panel_Tick( root );
}

void GUI_UpdateMousePosition( int x, int y )
{
	guiState.mouseOldPos = guiState.mousePos;
	guiState.mousePos.x = x;
	guiState.mousePos.y = y;
}

void GUI_UpdateMouseWheel( float x, float y )
{
	guiState.mouseOldWheel = guiState.mouseWheel;
	guiState.mouseWheel.x = x;
	guiState.mouseWheel.y = y;
}

void GUI_UpdateMouseButton( GUIMouseButton button, bool isDown )
{
}
