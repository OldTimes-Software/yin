// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_hashtable.h>

#include <plgraphics/plg_framebuffer.h>

#include "engine_private.h"
#include "renderer.h"

typedef struct YRRenderTarget
{
	char            id[ 16 ];// 'rt_menu_0'
	PLGTexture     *texture;
	PLGFrameBuffer *frameBuffer;
	MMReference     reference;
} YRRenderTarget;

static PLHashTable *renderTargets;

void YR_RenderTarget_Initialize( void )
{
	renderTargets = PlCreateHashTable();
	if ( renderTargets == NULL )
	{
		PRINT_ERROR( "Failed to create render target hash table: %s\n", PlGetError() );
	}
}

void YR_RenderTarget_Shutdown( void )
{
	MemoryManager_FlushUnreferencedResources();

	PLHashTableNode *node = PlGetFirstHashTableNode( renderTargets );
	while ( node != NULL )
	{
		YRRenderTarget *renderTarget = ( YRRenderTarget * ) PlGetHashTableNodeUserData( node );

		int numReferences = MemoryManager_GetNumberOfReferences( &renderTarget->reference );
		if ( numReferences > 0 )
		{
			PRINT( "%s with %u references on shutdown!\n", renderTarget->id, numReferences );
		}

		node = PlGetNextHashTableNode( renderTargets, node );
	}

	PlDestroyHashTable( renderTargets );
}

YRRenderTarget *YR_RenderTarget_GetByKey( const char *key )
{
	return ( YRRenderTarget * ) PlLookupHashTableUserData( renderTargets, key, strlen( key ) );
}

static PLGFrameBuffer *CreateFrameBuffer( unsigned int width, unsigned int height, unsigned int flags )
{
	PLGFrameBuffer *frameBuffer = PlgCreateFrameBuffer( width, height, flags );
	if ( frameBuffer == NULL )
	{
		PRINT_WARNING( "Failed to create specified framebuffer: %s\n", PlGetError() );
		return NULL;
	}

	return frameBuffer;
}

static void DestroyRenderTarget( void *user )
{
	YRRenderTarget *renderTarget = ( YRRenderTarget * ) user;
	PlgDestroyTexture( renderTarget->texture );
}

YRRenderTarget *YR_RenderTarget_Create( const char *key, unsigned int width, unsigned int height, unsigned int flags )
{
	// Check if it's already been created, and if so, update size to match
	YRRenderTarget *renderTarget = YR_RenderTarget_GetByKey( key );
	if ( renderTarget != NULL )
	{
		if ( flags == 0 )
		{
			PRINT_DEBUG( "Placeholder render target \"%s\" was already generated, returning existing\n", key );
			MemoryManager_AddReference( &renderTarget->reference );
			return renderTarget;
		}

		if ( renderTarget->frameBuffer == NULL )
		{
			renderTarget->frameBuffer = PlgCreateFrameBuffer( width, height, flags );
			if ( renderTarget->frameBuffer == NULL )
			{
				PRINT_WARNING( "Failed to create specified framebuffer for target \"%s\": %s\n", key, PlGetError() );
				return NULL;
			}
		}

		PRINT_DEBUG( "Render target already exists, updating size\n" );
		YR_RenderTarget_SetSize( renderTarget, width, height );
		MemoryManager_AddReference( &renderTarget->reference );
		return renderTarget;
	}

	PLGFrameBuffer *frameBuffer;
	if ( flags != 0 )
	{
		frameBuffer = CreateFrameBuffer( width, height, flags );
		if ( frameBuffer == NULL )
		{
			PRINT_WARNING( "Failed to create render target, \"%s\"\n", key );
			return NULL;
		}
	}
	else
	{
		PRINT_DEBUG( "Creating placeholder render target, \"%s\"\n", key );
		frameBuffer = NULL;
	}

	renderTarget              = PL_NEW( YRRenderTarget );
	renderTarget->frameBuffer = frameBuffer;
	snprintf( renderTarget->id, sizeof( renderTarget->id ), "%s", key );

	MemoryManager_SetupReference( "RenderTarget", MEM_CACHE_TEXTURES, &renderTarget->reference, DestroyRenderTarget, renderTarget );
	MemoryManager_AddReference( &renderTarget->reference );

	PlInsertHashTableNode( renderTargets, key, strlen( key ), renderTarget );

	return renderTarget;
}

void YR_RenderTarget_Release( YRRenderTarget *renderTarget )
{
	MemoryManager_ReleaseReference( &renderTarget->reference );
}

void YR_RenderTarget_SetSize( YRRenderTarget *renderTarget, unsigned int width, unsigned int height )
{
	PlgSetFrameBufferSize( renderTarget->frameBuffer, width, height );
	if ( PlGetFunctionResult() != PL_RESULT_SUCCESS )
		PRINT_WARNING( "Failed to resize framebuffer: %s\n", PlGetError() );
}

PLGTexture *YR_RenderTarget_GetTextureAttachment( YRRenderTarget *renderTarget )
{
	return renderTarget->texture;
}
