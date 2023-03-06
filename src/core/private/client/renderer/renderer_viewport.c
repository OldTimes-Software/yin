// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "core_private.h"
#include "renderer.h"

/**
 * What's a viewport? I hear you ask. Well, I'm glad you did.
 * When anything is drawn, a viewport handle needs to be provided,
 * this essentially represents what we're rendering *to* - i.e. a
 * window. Whenever the shell updates a particular window, it will
 * basically provide a viewport representing that window, which
 * in-turn allows the engine to track the frame-time for that
 * particular window.
 */

#define MAX_VIEWPORTS 4
static YNCoreViewport *viewports[ MAX_VIEWPORTS ] =
        { NULL, NULL, NULL, NULL };

/**
 * Attempts to create a new viewport. Only a maximum of 4 are supported.
 */
YNCoreViewport *YnCore_Viewport_Create( int x, int y, int width, int height, void *windowHandle )
{
	unsigned int i = 0;
	for ( ; i < MAX_VIEWPORTS; ++i )
	{
		if ( viewports[ i ] != NULL )
			continue;

		break;
	}

	if ( i >= MAX_VIEWPORTS )
	{
		PRINT_WARNING( "Hit maximum viewport limit! Viewport will not be created.\n" );
		return NULL;
	}

	viewports[ i ]               = PL_NEW( YNCoreViewport );
	viewports[ i ]->x            = x;
	viewports[ i ]->y            = y;
	viewports[ i ]->width        = width;
	viewports[ i ]->height       = height;
	viewports[ i ]->index        = i;
	viewports[ i ]->windowHandle = windowHandle;

	return viewports[ i ];
}

void YnCore_Viewport_Destroy( YNCoreViewport *viewport )
{
	if ( viewport == NULL )
		return;

	unsigned int index = viewport->index;
	PL_DELETE( viewports[ index ] );
	viewports[ index ] = NULL;
}

/**
 * Returns the viewport by the given slot.
 */
YNCoreViewport *YnCore_Viewport_GetBySlot( unsigned int slot )
{
	assert( slot < MAX_VIEWPORTS );
	if ( slot >= MAX_VIEWPORTS )
	{
		PRINT_WARNING( "Invalid slot specified!\n" );
		return NULL;
	}

	return viewports[ slot ];
}

void YnCore_Viewport_SetCamera( YNCoreViewport *viewport, YNCoreCamera *camera )
{
	viewport->camera = camera;
}

void YnCore_Viewport_SetSize( YNCoreViewport *viewport, int width, int height )
{
	viewport->width  = width;
	viewport->height = height;
}

void YnCore_Viewport_GetSize( const YNCoreViewport *viewport, int *width, int *height )
{
	*width  = viewport->width;
	*height = viewport->height;
}

/**
 * Weird one, I know, but frametime is tied in with each viewport...
 */
unsigned int YnCore_Viewport_GetAverageFPS( const YNCoreViewport *viewport )
{
	double t = 0.0;
	for ( unsigned int i = 0; i < YN_CORE_MAX_FPS_READINGS; ++i )
		t += viewport->perf.frameReadings[ i ];

	return ( unsigned int ) ( t / YN_CORE_MAX_FPS_READINGS );
}
