// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "engine_private.h"
#include "editor.h"

#include "client/renderer/renderer.h"
#include "client/renderer/renderer_material.h"
#include "client/renderer/font.h"

static Material   **materials;
static unsigned int numMaterials, maxMaterials;

#define MATERIAL_DEFAULT_WIDTH 128

#define MATERIAL_STORE_INC 256

static void CacheMaterialPreviewCallback( const char *path, void *user )
{
	Material *material = YinCore_Material_Cache( path, CACHE_GROUP_WORLD, false, true );
	if ( material == NULL )
	{
		return;
	}

	materials[ numMaterials++ ] = material;
	if ( numMaterials >= maxMaterials )
	{
		maxMaterials += MATERIAL_STORE_INC;
		materials = PlReAllocA( materials, sizeof( Material * ) * maxMaterials );
	}
}

static int CompareMaterials( const void *a, const void *b )
{
	const char *strA = Eng_Ren_Material_GetPath( ( Material * ) a );
	const char *strB = Eng_Ren_Material_GetPath( ( Material * ) b );
	return strcmp( strA, strB );
}

void Editor_MaterialSelector_Initialize( void )
{
	numMaterials = 0;
	maxMaterials = MATERIAL_STORE_INC;
	materials = PL_NEW_( Material *, maxMaterials );

	// Cache all the materials in a preview state
	PlScanDirectory( "materials/world/", "n", CacheMaterialPreviewCallback, true, NULL );
	PRINT( "Found %u world materials\n", numMaterials );

	qsort( materials, numMaterials, sizeof( Material * ), CompareMaterials );
	for ( unsigned int i = 0; i < numMaterials; ++i )
	{
		PRINT( "\t%s\n", Eng_Ren_Material_GetPath( materials[ i ] ) );
	}
}

void Editor_MaterialSelector_Shutdown( void )
{
	for ( unsigned int i = 0; i < numMaterials; ++i )
	{
		YinCore_Material_Release( materials[ i ] );
	}
	PL_DELETE( materials );
}

/**
 * Draw the material selection panel.
 */
void Editor_MaterialSelector_Draw( const YRViewport *viewport )
{
	return;
	static const unsigned int mw = MATERIAL_DEFAULT_WIDTH;
	static const unsigned int mh = MATERIAL_DEFAULT_WIDTH;
	static const unsigned int sp = MATERIAL_DEFAULT_WIDTH / 8;

	unsigned int x = sp;
	unsigned int y = sp;

	PlMatrixMode( PL_MODELVIEW_MATRIX );
	PlPushMatrix();
	PlLoadIdentityMatrix();

	PlgSetShaderProgram( defaultShaderPrograms[ RS_SHADER_DEFAULT_VERTEX ] );

	PlgDrawRectangle( 0, 0, ( float ) viewport->width, ( float ) viewport->height, PL_COLOUR_DARK_SLATE_GRAY );

	PlgSetShaderProgram( defaultShaderPrograms[ RS_SHADER_DEFAULT ] );

	BitmapFont *font = Font_GetDefaultSmall();
	for ( unsigned int i = 0; i < numMaterials; ++i )
	{
		PLGTexture *texture = RM_GetPreviewTexture( materials[ i ] );
		PlgDrawTexturedRectangle( ( float ) x, ( float ) y, ( float ) mw, ( float ) mh, texture );

		char buf[ 8 ];
		snprintf( buf, sizeof( buf ), "%ux%u", texture->w, texture->h );
		Font_DrawBitmapString( font, ( float ) ( x + sp ), ( float ) ( y + sp ), 1.0f, 1.0f, PL_COLOUR_WHITE, buf, true );
		Font_DrawBitmapString( font, ( float ) x, ( float ) ( y + mw + 2 ), 1.0f, 1.0f, PL_COLOUR_WHITE, Eng_Ren_Material_GetPath( materials[ i ] ), true );

		x += mw + sp;
		if ( x + mw >= viewport->width )
		{
			x = sp;
			y += sp + mh;
		}

		if ( y >= viewport->height )
			break;
	}

	PlPopMatrix();
}

void Editor_MaterialSelector_Tick( void )
{
}
