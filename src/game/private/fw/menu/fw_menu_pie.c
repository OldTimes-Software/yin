// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "fw_menu_pie.h"

#define PIE_MENU_WIDTH         128
#define PIE_MENU_HEIGHT        128
#define PIE_MENU_OPTION_WIDTH  128
#define PIE_MENU_OPTION_HEIGHT 128

typedef struct FWPieMenu
{
	PLLinkedList *options;
	PLLinkedListNode *activeOption;// option we're currently selecting
	PLLinkedListNode *targetOption;// option we want to be at, for animating
	bool isActive;
	float angle, scale, velocity;
	PLVector2 cursor;
	int w, h;
} FWPieMenu;

typedef struct FWPieMenuOption
{
	PLLinkedListNode *node;
	char label[ 64 ];
	struct YNCoreMaterial *icon;
	FWPieMenuOptionCallback callback;
	FWPieMenu *parent;
} FWPieMenuOption;

FWPieMenu *FW_Menu_CreatePie( void )
{
	// don't need to do much here, just allocate and return
	FWPieMenu *menu = PL_NEW( FWPieMenu );
	menu->options   = PlCreateLinkedList();
	menu->w         = PIE_MENU_WIDTH;
	menu->h         = PIE_MENU_HEIGHT;
	return menu;
}

void FW_Menu_DestroyPie( FWPieMenu *menu )
{
	if ( menu == NULL )
		return;

	// destroy all the pie options
	PLLinkedListNode *node = PlGetFirstNode( menu->options );
	while ( node != NULL )
	{
		PL_DELETE( PlGetLinkedListNodeUserData( node ) );
		node = PlGetNextLinkedListNode( node );
	}
	PlDestroyLinkedList( menu->options );

	PL_DELETE( menu );
}

void FW_Menu_TickPie( FWPieMenu *menu )
{
	//menu->angle += 0.5f;

	if ( menu->velocity != 0 )
		menu->velocity -= menu->velocity / 8.0f;
	menu->angle += menu->velocity;

	if ( menu->scale < 1.0f )
		menu->scale += 0.05f;
}

bool FW_Menu_HandlePieInput( FWPieMenu *menu )
{
	if ( !menu->isActive )
		return false;

	// if the active option is null, reset it to the first slot
	if ( menu->activeOption == NULL )
	{
		menu->activeOption = PlGetFirstNode( menu->options );
		if ( menu->activeOption == NULL )
			// probably no options available, just return...
			return false;
	}

#if 0// why would you do a pie menu this way... ? dummy
	// cycle clockwise around the list
	if ( Eng_Cli_Input_GetButtonStatus( 0, INPUT_LEFT ) == INPUT_STATE_PRESSED )
	{
		Game_Debug( "Select left...\n" );

		if ( menu->targetOption == NULL )
			menu->targetOption = PlGetNextLinkedListNode( menu->activeOption );
		else
			menu->targetOption = PlGetNextLinkedListNode( menu->targetOption );

		if ( menu->targetOption == NULL )
			menu->targetOption = PlGetFirstNode( menu->options );

		return true;
	}
	// counter-clockwise
	else if ( Eng_Cli_Input_GetButtonStatus( 0, INPUT_RIGHT ) == INPUT_STATE_PRESSED )
	{
		Game_Debug( "Select right...\n" );

		if ( menu->targetOption == NULL )
			menu->targetOption = PlGetPrevLinkedListNode( menu->activeOption );
		else
			menu->targetOption = PlGetPrevLinkedListNode( menu->targetOption );

		if ( menu->targetOption == NULL )
			menu->targetOption = PlGetLastNode( menu->options );

		return true;
	}
	// select
	else
#endif

	PLVector2 joyPos = YnCore_Input_GetStickStatus( 0, 0 );
	menu->cursor     = joyPos;

	if ( YnCore_Input_GetButtonStatus( 0, INPUT_A ) == YN_CORE_INPUT_STATE_PRESSED )
	{
		Game_Debug( "Selected item...\n" );

		FWPieMenuOption *option;
		if ( menu->targetOption != NULL )
			// option we're trying to get to
			option = PlGetLinkedListNodeUserData( menu->targetOption );
		else if ( menu->activeOption != NULL )
			// option we're currently at
			option = PlGetLinkedListNodeUserData( menu->activeOption );
		else
			// nothing to select!
			return true;

		if ( option->callback == NULL )
			return true;

		option->callback( menu, option, NULL );
		return true;
	}

	if ( YnCore_Input_GetButtonStatus( 0, INPUT_RB ) != YN_CORE_INPUT_STATE_NONE )
	{
		menu->velocity -= 1.5f;
		return true;
	}
	else if ( YnCore_Input_GetButtonStatus( 0, INPUT_LB ) != YN_CORE_INPUT_STATE_NONE )
	{
		menu->velocity += 1.5f;
		return true;
	}

	return false;
}

static void DrawPieOption( FWPieMenuOption *option, float x, float y, bool isSelected, float scale )
{
	float w = PIE_MENU_OPTION_WIDTH * scale;
	float h = PIE_MENU_OPTION_HEIGHT * scale;

	PLColour colour = PL_COLOUR_WHITE;

	x -= w / 2;
	y -= h / 2;

	PLGMesh *mesh = PlgImmBegin( PLG_MESH_TRIANGLE_STRIP );

	PlgImmPushVertex( x, y, 0.0f );
	PlgImmTextureCoord( 0.0f, 0.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );

	PlgImmPushVertex( x, y + h, 0.0f );
	PlgImmTextureCoord( 0.0f, 1.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );

	PlgImmPushVertex( x + w, y, 0.0f );
	PlgImmTextureCoord( 1.0f, 0.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );

	PlgImmPushVertex( x + w, y + h, 0.0f );
	PlgImmTextureCoord( 1.0f, 1.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );

	if ( option->icon == NULL )
	{
		PlgImmDraw();
		return;
	}

	YnCore_Material_DrawMesh( option->icon, mesh, NULL, 0 );
}

static FWPieMenuOption *GetSelectedOption( FWPieMenu *menu )
{
	if ( menu->targetOption != NULL )
		return PlGetLinkedListNodeUserData( menu->targetOption );
	if ( menu->activeOption != NULL )
		return PlGetLinkedListNodeUserData( menu->activeOption );

	return NULL;
}

static void GetOptionAngle( FWPieMenu *menu, FWPieMenuOption *option )
{
}

void FW_Menu_DrawPie( FWPieMenu *menu, float x, float y )
{
	if ( !menu->isActive )
		return;

	float cursorX = x + ( menu->cursor.x * ( ( float ) menu->w / 2.0f ) );
	float cursorY = y + ( menu->cursor.y * ( ( float ) menu->h / 2.0f ) );

	unsigned int numElements = PlGetNumLinkedListNodes( menu->options );
	PLLinkedListNode *node   = PlGetFirstNode( menu->options );
	for ( unsigned int i = 0, pos = 0; i < 360; i += ( 360 / numElements ) )
	{
		if ( pos >= numElements || node == NULL )
			break;

		// absolute screen coordinate of the pie option element
		float xo = x + cosf( PL_DEG2RAD( ( float ) i + menu->angle ) ) * ( ( ( float ) menu->w / 2.0f ) * menu->scale );
		float yo = y + sinf( PL_DEG2RAD( ( float ) i + menu->angle ) ) * ( ( ( float ) menu->h / 2.0f ) * menu->scale );

		// determine the distance between the cursor and the pie option element
		float xc = fabsf( cursorX - xo ) / ( float ) menu->w;
		float yc = fabsf( cursorY - yo ) / ( float ) menu->h;

		// and now the scale
		float dc = menu->scale * ( PlGetVector2Length( &PLVector2( 1.0f, 1.0f ) ) - PlGetVector2Length( &PLVector2( xc, yc ) ) );

		FWPieMenuOption *option = PlGetLinkedListNodeUserData( node );
		bool isSelected         = ( option == GetSelectedOption( menu ) );
		DrawPieOption( option, xo, yo, isSelected, dc );
		if ( isSelected )
			PlgDrawSimpleLine( PlMatrix4Identity(), PLVector3( x, y, 0.0f ), PLVector3( xo, yo, 0.0f ), PL_COLOUR_RED );

		node = PlGetNextLinkedListNode( node );
	}

	DrawPieOption( PlGetLinkedListNodeUserData( PlGetFirstNode( menu->options ) ), cursorX, cursorY, false, 1.0f );
}

void FW_Menu_SetPieActive( FWPieMenu *menu, bool active )
{
	menu->isActive = active;
	menu->scale    = 0.0f;
}

FWPieMenuOption *FW_Menu_AddPieOption( FWPieMenu *menu, const char *label, struct YNCoreMaterial *icon, FWPieMenuOptionCallback callback )
{
	FWPieMenuOption *option = PL_NEW( FWPieMenuOption );
	option->node            = PlInsertLinkedListNode( menu->options, option );
	option->callback        = callback;
	option->icon            = icon;
	option->parent          = menu;
	snprintf( option->label, sizeof( option->label ), "%s", label );

	menu->w += 32;
	menu->h += 32;

	// if we have no active option, set it to this
	if ( menu->activeOption == NULL )
		menu->activeOption = PlGetFirstNode( menu->options );

	return option;
}

void FW_Menu_DestroyPieOption( FWPieMenuOption *option )
{
	FWPieMenu *menu = option->parent;
	assert( menu != NULL );
	if ( menu != NULL )
	{
		// Just reset the target option back to NULL
		if ( menu->targetOption != NULL )
			menu->targetOption = NULL;
		// And reset the active option to the first node if it's the same as our selection
		if ( menu->activeOption != NULL && ( ( FWPieMenuOption * ) PlGetLinkedListNodeUserData( menu->activeOption ) ) == option )
			menu->activeOption = PlGetFirstNode( menu->options );

		menu->w -= 32;
		menu->h -= 32;
	}
	else
		Game_Warning( "Encountered a pie option with no parent!\n" );

	if ( option->icon != NULL )
		YnCore_Material_Release( option->icon );

	PL_DELETE( option );
}
