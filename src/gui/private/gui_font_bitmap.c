// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "gui_private.h"

/****************************************
 * GUI BITMAP FONT API
 ****************************************/

typedef struct GUIBitmapFont
{
	PLGTexture  *texture;
	int          cw, ch;
	unsigned int start, end;
} GUIBitmapFont;

static GUIBitmapFont *defaultFont;
static GUIBitmapFont *defaultSmallFont;

static PLLinkedList *cachedFonts;

GUIBitmapFont *GUI_BitmapFont_CacheFont( const char *path, int w, int h, int cw, int ch, unsigned int start, unsigned int end )
{
	PLGTexture *texture = GUI_CacheTexture( path );
	if ( texture == NULL )
		return NULL;

	GUIBitmapFont *font = PL_NEW( GUIBitmapFont );
	font->texture = texture;
	font->cw = cw;
	font->ch = ch;
	font->start = start;
	font->end = end;

	return font;
}

void GUI_BitmapFont_Initialize( void )
{
	cachedFonts = PlCreateLinkedList();

	//defaultFont = GUI_BitmapFont_CacheFont( "guis/fonts/default.png", 256, 48, 8, 12, 0, 128 );
	//defaultSmallFont = GUI_BitmapFont_CacheFont( "guis/fonts/default_small.png", 128, 24, 4, 6, 0, 128 );
}
