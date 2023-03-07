// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_physics.h>

PL_EXTERN_C

/* external elements */
typedef struct YNCoreCamera YNCoreCamera;
typedef struct YNCoreViewport YNCoreViewport;

/* ======================================================================
 * WORLD INTERFACE
 * ====================================================================*/

typedef struct YNCoreWorldFace YNCoreWorldFace;
typedef struct YNCoreWorldMesh YNCoreWorldMesh;
typedef struct YNCoreWorldObject YNCoreWorldObject;
typedef struct YNCoreWorldSector YNCoreWorldSector;
typedef struct YNCoreWorld YNCoreWorld;

#define YN_CORE_WORLD_VERSION 2

#define YN_CORE_WORLD_EXTENSION      "wld.n"
#define YN_CORE_WORLD_EXTENSION_MESH "wsm.n"

/* World */

YNCoreWorld *YnCore_World_Create( void );
YNCoreWorld *YnCore_World_Load( const char *path );

/**
 * Attempts to save the given world to the destination.
 * On success, returns true but false otherwise.
 */
bool YnCore_World_Save( YNCoreWorld *world, const char *path );

void YnCore_World_Destroy( YNCoreWorld *world );
struct YNNodeBranch *YnCore_World_GetProperty( YNCoreWorld *world, const char *propertyName );
PLColourF32 YnCore_World_GetAmbience( YNCoreWorld *world );
PLColourF32 YnCore_World_GetSunColour( YNCoreWorld *world );
PLVector3 YnCore_World_GetSunPosition( YNCoreWorld *world );
void YnCore_World_DrawWireframe( YNCoreWorld *world, YNCoreCamera *camera );
void YnCore_World_Draw( YNCoreWorld *world, YNCoreWorldSector *originSector, YNCoreCamera *camera );
void YnCore_World_SetupGlobalDefaults( YNCoreWorld *world );

uint64_t YnCore_World_GetLastSaveTime( const YNCoreWorld *world );

YNCoreWorldSector *YnCore_World_GetSectorByGlobalOrigin( YNCoreWorld *world, const PLVector3 *globalOrigin );

const char *YnCore_World_GetPath( const YNCoreWorld *world );

/* Mesh */

YNCoreWorldMesh *YnCore_WorldMesh_Create( YNCoreWorld *parent );
YNCoreWorldMesh *YnCore_WorldMesh_Load( const char *path );
void YnCore_WorldMesh_Release( YNCoreWorldMesh *worldMesh );

/* Face */

PLVector3 YnCore_WorldFace_GetNormal( const YNCoreWorldFace *face );
PLVector3 YnCore_WorldFace_GetOrigin( const YNCoreWorldFace *face );
uint8_t YnCore_WorldFace_GetFlags( const YNCoreWorldFace *face );
const PLCollisionAABB *YnCore_WorldFace_GetBounds( const YNCoreWorldFace *face );

/* Sector */

struct YNCoreLight *YnCore_WorldSector_GetVisibleLights( YNCoreWorldSector *sector, unsigned int *numLights );
YNCoreWorldMesh *YnCore_WorldSector_GetMesh( YNCoreWorldSector *sector );
YNCoreWorldFace **YnCore_WorldSector_GetMeshFaces( YNCoreWorldSector *sector, uint32_t *numFaces );

PL_EXTERN_C_END
