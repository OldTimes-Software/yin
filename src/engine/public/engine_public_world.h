/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_physics.h>

PL_EXTERN_C

/* external elements */
typedef struct YRCamera YRCamera;
typedef struct YRViewport YRViewport;

/* ======================================================================
 * WORLD INTERFACE
 * ====================================================================*/

typedef struct WorldFace WorldFace;
typedef struct WorldMesh WorldMesh;
typedef struct WorldObject WorldObject;
typedef struct WorldSector WorldSector;
typedef struct World World;

#define WORLD_VERSION 2

#define WORLD_EXTENSION      "wld.n"
#define WORLD_EXTENSION_MESH "wsm.n"

/* World */

World *World_Create( void );
World *World_Load( const char *path );

/**
 * Attempts to save the given world to the destination.
 * On success, returns true but false otherwise.
 */
bool World_Save( World *world, const char *path );

void World_Destroy( World *world );
struct NLNode *World_GetProperty( World *world, const char *propertyName );
PLColourF32 World_GetAmbience( World *world );
PLColourF32 World_GetSunColour( World *world );
PLVector3 World_GetSunPosition( World *world );
void YR_World_DrawWireframe( World *world, YRCamera *camera );
void YR_World_Draw( World *world, WorldSector *originSector, YRCamera *camera );
void World_SetupGlobalDefaults( World *world );

uint64_t World_GetLastSaveTime( const World *world );

WorldSector *World_GetSectorByGlobalOrigin( World *world, const PLVector3 *globalOrigin );

const char *World_GetPath( const World *world );

/* Mesh */

WorldMesh *World_Mesh_Create( World *parent );
WorldMesh *World_Mesh_Load( const char *path );
void World_Mesh_Release( WorldMesh *worldMesh );

/* Face */

PLVector3 WorldFace_GetNormal( const WorldFace *face );
PLVector3 WorldFace_GetOrigin( const WorldFace *face );
uint8_t WorldFace_GetFlags( const WorldFace *face );
const PLCollisionAABB *WorldFace_GetBounds( const WorldFace *face );

/* Sector */

struct OSLight *World_Sector_GetVisibleLights( WorldSector *sector, unsigned int *numLights );
WorldMesh *WorldSector_GetMesh( WorldSector *sector );
WorldFace **WorldSector_GetMeshFaces( WorldSector *sector, uint32_t *numFaces );

PL_EXTERN_C_END
