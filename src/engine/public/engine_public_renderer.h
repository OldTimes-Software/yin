// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plgraphics/plg_mesh.h>

typedef struct YRCamera YRCamera;
typedef struct YRViewport YRViewport;
typedef struct OSLight OSLight;

typedef struct Material Material;

// TODO: retire this...
typedef enum CacheGroup
{
	CACHE_GROUP_WORLD, /* everything that is cached during level load */
	MAX_CACHE_GROUPS
} CacheGroup;

PL_EXTERN_C

YRViewport *YinCore_Viewport_Create( int x, int y, int width, int height, void *windowHandle );
void YinCore_Viewport_Destroy( YRViewport *viewport );
YRViewport *YinCore_Viewport_GetBySlot( unsigned int slot );
void YinCore_Viewport_SetCamera( YRViewport *viewport, YRCamera *camera );
void YinCore_Viewport_SetSize( YRViewport *viewport, int width, int height );
void YinCore_Viewport_GetSize( const YRViewport *viewport, int *width, int *height );
unsigned int YinCore_Viewport_GetAverageFPS( const YRViewport *viewport );

// materials

/**
 * Returns the original path the material was loaded from.
 */
const char *Eng_Ren_Material_GetPath( const Material *material );

/**
 * Returns the filename for the material.
 */
const char *Eng_Ren_Material_GetName( const Material *material );

/**
 * Cache a new material into memory if not so already, otherwise
 * returns an existing material from the cache and adds a reference -
 * reference will need to be released once finished with.
 */
Material *YinCore_Material_Cache( const char *path, CacheGroup group, bool useFallback, bool preview );

/**
 * Releases a reference to the material, allowing it to clean up.
 */
void YinCore_Material_Release( Material *material );

/**
 * Draws the given mesh with the given material. This also updates the peformance tracking,
 * so ideally you should always use this when drawing any mesh.
 */
void YinCore_Material_DrawMesh( Material *material, PLGMesh *mesh, OSLight *lights, unsigned int numLights );

PL_EXTERN_C_END
