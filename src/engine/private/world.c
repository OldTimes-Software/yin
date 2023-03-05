// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_filesystem.h>

#include <plgraphics/plg.h>
#include <plgraphics/plg_mesh.h>

#include "engine_private.h"
#include "world.h"
#include "legacy/actor.h"

#include "node/public/node.h"

#include "client/renderer/renderer.h"
#include "client/renderer/renderer_material.h"
#include "engine_public_world.h"

void World_SetupGlobalDefaults( World *world )
{
	world->ambience    = WORLD_DEFAULT_AMBIENCE;
	world->sunColour   = WORLD_DEFAULT_SUNCOLOUR;
	world->sunPosition = WORLD_DEFAULT_SUNPOSITION;
	world->clearColour = WORLD_DEFAULT_CLEARCOLOUR;
}

World *World_Create( void )
{
	World *world = PlMAllocA( sizeof( World ) );

	world->globalProperties = NL_PushBackObj( NULL, "properties" );
	NL_PushBackF32Array( world->globalProperties, "ambience", ( const float * ) &WORLD_DEFAULT_AMBIENCE, 4 );
	NL_PushBackF32Array( world->globalProperties, "clearColour", ( const float * ) &WORLD_DEFAULT_CLEARCOLOUR, 4 );
	//NL_PushBackStrArray( world->globalProperties, "skyMaterials", ( const char ** ) WORLD_DEFAULT_SKY, 1 );

	world->meshes   = PlCreateVectorArray( 0 );
	world->entities = PlCreateLinkedList();

	return world;
}

World *World_Load( const char *path )
{
	NLNode *node = NL_LoadFile( path, "world" );
	if ( node == NULL )
	{
		PRINT_WARNING( "Failed to load world: %s\n", path );
		return NULL;
	}

	World *world = World_Create();
	snprintf( world->path, sizeof( world->path ), "%s", path );
	if ( WorldDeserialiser_Begin( node, world ) == NULL )
	{
		World_Destroy( world );
		world = NULL;
	}

	NL_DestroyNode( node );

	return world;
}

bool World_Save( World *world, const char *path )
{
	world->lastSaveTime = time( NULL );

	NLNode *root = NL_PushBackObj( NULL, "world" );

	WorldSerialiser_Begin( world, root );
	snprintf( world->path, sizeof( world->path ), "%s", path );

	if ( !NL_WriteFile( path, root, NL_FILE_BINARY ) )
	{
		PRINT_WARNING( "Failed to save world (%s): %s\n", path, NL_GetErrorMessage() );
		return false;
	}

	return true;
}

/**
 * Clears the current assigned mesh and all static
 * objects for the given sector.
 */
static void ClearSector( WorldSector *sector )
{
	for ( unsigned int i = 0; i < sector->numStaticObjects; ++i )
		World_Mesh_Release( sector->staticObjects[ i ].mesh );

	PlFree( sector->staticObjects );

	World_Mesh_Release( sector->mesh );
}

static void DestroyWorldEntities( World *world )
{
	PLLinkedListNode *node = PlGetFirstNode( world->entities );
	while ( node != NULL )
	{
		PL_DELETE( PlGetLinkedListNodeUserData( node ) );
		node = PlGetNextLinkedListNode( node );
	}
}

void World_Destroy( World *world )
{
	if ( world == NULL )
	{
		return;
	}

	for ( unsigned int i = 0; i < world->numSectors; ++i )
	{
		ClearSector( &world->sectors[ i ] );
	}

	PlFree( world->sectors );

	unsigned int numMeshes = PlGetNumVectorArrayElements( world->meshes );
	for ( unsigned int i = 0; i < numMeshes; ++i )
	{
		World_Mesh_Release( ( WorldMesh * ) PlGetVectorArrayElementAt( world->meshes, i ) );
	}

	DestroyWorldEntities( world );

	PlDestroyVectorArray( world->meshes );
	PlFree( world );
}

PLLinkedList *World_GetLights( const World *world )
{
	for ( unsigned int i = 0; i < world->numSectors; ++i )
	{
	}

	return NULL;
}

PLLinkedList *World_GetSectorLights( const WorldSector *sector )
{
	return sector->lights;
}

void World_SpawnEntities( World *world )
{
	PLLinkedListNode *node = PlGetFirstNode( world->entities );
	while ( node != NULL )
	{
		WorldEntity *worldEntity = ( WorldEntity * ) PlGetLinkedListNodeUserData( node );
		YinCore_EntityManager_CreateEntityFromPrefab( worldEntity->entityTemplate->name );
		node = PlGetNextLinkedListNode( node );
	}
}

/****************************************
 * Global World Properties
 ****************************************/

NLNode *World_GetProperty( World *world, const char *propertyName )
{
	if ( world->globalProperties == NULL )
		return NULL;

	return NL_GetChildByName( world->globalProperties, propertyName );
}

PLColourF32 World_GetAmbience( World *world ) { return world->ambience; }
PLColourF32 World_GetSunColour( World *world ) { return world->sunColour; }
PLVector3 World_GetSunPosition( World *world ) { return world->sunPosition; }

/****************************************
 ****************************************/

uint64_t World_GetLastSaveTime( const World *world )
{
	return world->lastSaveTime;
}

/**
 * Fetch the normal for the specified face.
 */
PLVector3 WorldFace_GetNormal( const WorldFace *face )
{
	return face->normal;
}

/**
 * Fetch the origin point of the face in world-coordinates.
 */
PLVector3 WorldFace_GetOrigin( const WorldFace *face )
{
	return face->bounds.absOrigin;
}

/**
 * Fetch the flags specified for the face.
 */
uint8_t WorldFace_GetFlags( const WorldFace *face )
{
	return face->flags;
}

/****************************************
 * SECTOR
 ****************************************/

OSLight *World_Sector_GetVisibleLights( WorldSector *sector, unsigned int *numLights )
{
	// TODO: for now we're just going to return this static list...
	static OSLight lights[] = {
	        {
             .position = { 10.0f, 10.0f, 10.0f },
             .colour   = { 1.0f, 0.0f, 0.0f, 16.0f },
             .radius   = 16.0f,
	         },
	};

	*numLights = PL_ARRAY_ELEMENTS( lights );
	return lights;
}

/**
 * This crudely tries to determine the sector by an origin point.
 * Should only be used for vague lookup.
 */
WorldSector *World_GetSectorByGlobalOrigin( World *world, const PLVector3 *globalOrigin )
{
	for ( unsigned int i = 0; i < world->numSectors; ++i )
	{
		WorldSector *sector = &world->sectors[ i ];
		if ( !PlIsPointIntersectingAabb( &sector->bounds, *globalOrigin ) )
		{
			continue;
		}

		return sector;
	}

	return &world->sectors[ 0 ];
}

const char *World_GetPath( const World *world )
{
	return world->path;
}

/**
 * Get the primary mesh for the given sector, this
 * is essentially the sector's body.
 */
WorldMesh *WorldSector_GetMesh( WorldSector *sector )
{
	return sector->mesh;
}

WorldFace **WorldSector_GetMeshFaces( WorldSector *sector, uint32_t *numFaces )
{
	if ( sector->mesh == NULL )
	{
		*numFaces = 0;
		return NULL;
	}

	*numFaces         = PlGetNumLinkedListNodes( sector->mesh->faces );
	WorldFace **faces = PL_NEW_( WorldFace *, *numFaces );

	PLLinkedListNode *faceNode = PlGetFirstNode( sector->mesh->faces );
	for ( unsigned int i = 0; i < *numFaces; ++i )
	{
		faces[ i ] = ( WorldFace * ) PlGetLinkedListNodeUserData( faceNode );
		faceNode   = PlGetNextLinkedListNode( faceNode );
	}

	return faces;
}

static WorldSector **GetVisibleSectors( World *world, WorldSector *originSector, const YRCamera *camera, unsigned int *numSectors )
{
	PL_GET_CVAR( "world.drawSectors", drawSectors );
	if ( drawSectors != NULL && !drawSectors->b_value )
	{
		return NULL;
	}

	WorldSector **visibleSectors = PlCAllocA( world->numSectors, sizeof( WorldSector * ) );

	/* we'll assume the sector we're in is visible (seems like a safe assumption) */
	*numSectors                     = 0;
	visibleSectors[ *numSectors++ ] = originSector;

	/* todo
	 *  2. test whether or not player can see portal
	 *  3. get portal destination, do 2. again but from portal
	 */

#if 0
	WorldMesh *sectorMesh = originSector->mesh;
	if ( sectorMesh != NULL )
	{
		for ( unsigned int i = 0; i < sectorMesh->numFaces; ++i )
		{
			if ( !( sectorMesh->faces[ i ].flags & WORLD_FACE_FLAG_PORTAL ) && !( sectorMesh->faces[ i ].flags & WORLD_FACE_FLAG_MIRROR ) )
			{
				continue;
			}

			if ( !PlgIsBoxInsideView( camera->internal, &sectorMesh->faces[ i ].bounds ) )
			{
				continue;
			}
		}
	}
#endif

	return visibleSectors;
}

static WorldMesh **GetVisibleSubMeshesForSector( WorldSector *sector, const PLGCamera *camera, unsigned int *numMeshes )
{
	PL_GET_CVAR( "world.drawSubMeshes", drawSubMeshes );
	if ( drawSubMeshes != NULL && !drawSubMeshes->b_value )
		return NULL;

	WorldMesh **visibleMeshes = PlCAlloc( sector->numStaticObjects, sizeof( WorldMesh * ), true );

	// Go through and generate a list of visible meshes within the sector
	for ( unsigned int i = 0; i < sector->numStaticObjects; ++i )
	{
		PLCollisionAABB *bounds = &sector->staticObjects[ i ].mesh->bounds;
		bounds->origin          = sector->staticObjects[ i ].transform.translation;
		if ( !PlgIsBoxInsideView( camera, bounds ) )
		{
			continue;
		}

		visibleMeshes[ *numMeshes ] = sector->mesh;
		numMeshes++;
	}

	// Shrink the array down before passing it back
	visibleMeshes = PlReAlloc( visibleMeshes, sizeof( WorldMesh * ) * *numMeshes, true );
	return visibleMeshes;
}

/**
 * This is a little bit silly, but we're considering mirrors as a valid portal too...
 */
bool World_IsFacePortal( const WorldFace *face )
{
	return ( ( face->flags & WORLD_FACE_FLAG_MIRROR ) || ( face->flags & WORLD_FACE_FLAG_PORTAL ) );
}

WorldSector *World_GetSectorByNum( World *world, int sectorNum )
{
	if ( sectorNum < 0 || sectorNum >= world->numSectors )
		return NULL;

	return &world->sectors[ sectorNum ];
}
