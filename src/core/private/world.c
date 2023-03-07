// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_filesystem.h>
#include <plgraphics/plg_mesh.h>

#include "core_private.h"
#include "world.h"
#include "legacy/actor.h"

#include <yin/node.h>

#include "client/renderer/renderer.h"

void YnCore_World_SetupGlobalDefaults( YNCoreWorld *world )
{
	world->ambience    = WORLD_DEFAULT_AMBIENCE;
	world->sunColour   = WORLD_DEFAULT_SUNCOLOUR;
	world->sunPosition = WORLD_DEFAULT_SUNPOSITION;
	world->clearColour = WORLD_DEFAULT_CLEARCOLOUR;
}

YNCoreWorld *YnCore_World_Create( void )
{
	YNCoreWorld *world = PlMAllocA( sizeof( YNCoreWorld ) );

	world->globalProperties = YnNode_PushBackObject( NULL, "properties" );
	YnNode_PushBackF32Array( world->globalProperties, "ambience", ( const float * ) &WORLD_DEFAULT_AMBIENCE, 4 );
	YnNode_PushBackF32Array( world->globalProperties, "clearColour", ( const float * ) &WORLD_DEFAULT_CLEARCOLOUR, 4 );
	//NL_PushBackStrArray( world->globalProperties, "skyMaterials", ( const char ** ) WORLD_DEFAULT_SKY, 1 );

	world->meshes   = PlCreateVectorArray( 0 );
	world->entities = PlCreateLinkedList();

	return world;
}

YNCoreWorld *YnCore_World_Load( const char *path )
{
	YNNodeBranch *node = YnNode_LoadFile( path, "world" );
	if ( node == NULL )
	{
		PRINT_WARNING( "Failed to load world: %s\n", path );
		return NULL;
	}

	YNCoreWorld *world = YnCore_World_Create();
	snprintf( world->path, sizeof( world->path ), "%s", path );
	if ( YnCore_WorldDeserialiser_Begin( node, world ) == NULL )
	{
		YnCore_World_Destroy( world );
		world = NULL;
	}

	YnNode_DestroyBranch( node );

	return world;
}

bool YnCore_World_Save( YNCoreWorld *world, const char *path )
{
	world->lastSaveTime = time( NULL );

	YNNodeBranch *root = YnNode_PushBackObject( NULL, "world" );

	YnCore_WorldSerialiser_Begin( world, root );
	snprintf( world->path, sizeof( world->path ), "%s", path );

	if ( !YnNode_WriteFile( path, root, YN_NODE_FILE_BINARY ) )
	{
		PRINT_WARNING( "Failed to save world (%s): %s\n", path, YnNode_GetErrorMessage() );
		return false;
	}

	return true;
}

/**
 * Clears the current assigned mesh and all static
 * objects for the given sector.
 */
static void ClearSector( YNCoreWorldSector *sector )
{
	for ( unsigned int i = 0; i < sector->numStaticObjects; ++i )
		YnCore_WorldMesh_Release( sector->staticObjects[ i ].mesh );

	PlFree( sector->staticObjects );

	YnCore_WorldMesh_Release( sector->mesh );
}

static void DestroyWorldEntities( YNCoreWorld *world )
{
	PLLinkedListNode *node = PlGetFirstNode( world->entities );
	while ( node != NULL )
	{
		PL_DELETE( PlGetLinkedListNodeUserData( node ) );
		node = PlGetNextLinkedListNode( node );
	}
}

void YnCore_World_Destroy( YNCoreWorld *world )
{
	if ( world == NULL )
		return;

	for ( unsigned int i = 0; i < world->numSectors; ++i )
		ClearSector( &world->sectors[ i ] );

	PlFree( world->sectors );

	unsigned int numMeshes = PlGetNumVectorArrayElements( world->meshes );
	for ( unsigned int i = 0; i < numMeshes; ++i )
		YnCore_WorldMesh_Release( ( YNCoreWorldMesh * ) PlGetVectorArrayElementAt( world->meshes, i ) );

	DestroyWorldEntities( world );

	PlDestroyVectorArray( world->meshes );
	PlFree( world );
}

PLLinkedList *YnCore_World_GetLights( const YNCoreWorld *world )
{
	for ( unsigned int i = 0; i < world->numSectors; ++i )
	{
	}

	return NULL;
}

PLLinkedList *YnCore_World_GetSectorLights( const YNCoreWorldSector *sector )
{
	return sector->lights;
}

void YnCore_World_SpawnEntities( YNCoreWorld *world )
{
	PLLinkedListNode *node = PlGetFirstNode( world->entities );
	while ( node != NULL )
	{
		YNCoreWorldEntity *worldEntity = ( YNCoreWorldEntity * ) PlGetLinkedListNodeUserData( node );
		YnCore_EntityManager_CreateEntityFromPrefab( worldEntity->entityTemplate->name );
		node = PlGetNextLinkedListNode( node );
	}
}

/****************************************
 * Global World Properties
 ****************************************/

YNNodeBranch *YnCore_World_GetProperty( YNCoreWorld *world, const char *propertyName )
{
	if ( world->globalProperties == NULL )
		return NULL;

	return YnNode_GetChildByName( world->globalProperties, propertyName );
}

PLColourF32 YnCore_World_GetAmbience( YNCoreWorld *world ) { return world->ambience; }
PLColourF32 YnCore_World_GetSunColour( YNCoreWorld *world ) { return world->sunColour; }
PLVector3 YnCore_World_GetSunPosition( YNCoreWorld *world ) { return world->sunPosition; }

/****************************************
 ****************************************/

uint64_t YnCore_World_GetLastSaveTime( const YNCoreWorld *world )
{
	return world->lastSaveTime;
}

/**
 * Fetch the normal for the specified face.
 */
PLVector3 YnCore_WorldFace_GetNormal( const YNCoreWorldFace *face )
{
	return face->normal;
}

/**
 * Fetch the origin point of the face in world-coordinates.
 */
PLVector3 YnCore_WorldFace_GetOrigin( const YNCoreWorldFace *face )
{
	return face->bounds.absOrigin;
}

/**
 * Fetch the flags specified for the face.
 */
uint8_t YnCore_WorldFace_GetFlags( const YNCoreWorldFace *face )
{
	return face->flags;
}

/****************************************
 * SECTOR
 ****************************************/

YNCoreLight *YnCore_WorldSector_GetVisibleLights( YNCoreWorldSector *sector, unsigned int *numLights )
{
	// TODO: for now we're just going to return this static list...
	static YNCoreLight lights[] = {
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
YNCoreWorldSector *YnCore_World_GetSectorByGlobalOrigin( YNCoreWorld *world, const PLVector3 *globalOrigin )
{
	for ( unsigned int i = 0; i < world->numSectors; ++i )
	{
		YNCoreWorldSector *sector = &world->sectors[ i ];
		if ( !PlIsPointIntersectingAabb( &sector->bounds, *globalOrigin ) )
			continue;

		return sector;
	}

	return &world->sectors[ 0 ];
}

const char *YnCore_World_GetPath( const YNCoreWorld *world )
{
	return world->path;
}

/**
 * Get the primary mesh for the given sector, this
 * is essentially the sector's body.
 */
YNCoreWorldMesh *YnCore_WorldSector_GetMesh( YNCoreWorldSector *sector )
{
	return sector->mesh;
}

YNCoreWorldFace **YnCore_WorldSector_GetMeshFaces( YNCoreWorldSector *sector, uint32_t *numFaces )
{
	if ( sector->mesh == NULL )
	{
		*numFaces = 0;
		return NULL;
	}

	*numFaces         = PlGetNumLinkedListNodes( sector->mesh->faces );
	YNCoreWorldFace **faces = PL_NEW_( YNCoreWorldFace *, *numFaces );

	PLLinkedListNode *faceNode = PlGetFirstNode( sector->mesh->faces );
	for ( unsigned int i = 0; i < *numFaces; ++i )
	{
		faces[ i ] = ( YNCoreWorldFace * ) PlGetLinkedListNodeUserData( faceNode );
		faceNode   = PlGetNextLinkedListNode( faceNode );
	}

	return faces;
}

static YNCoreWorldSector **GetVisibleSectors( YNCoreWorld *world, YNCoreWorldSector *originSector, const YNCoreCamera *camera, unsigned int *numSectors )
{
	PL_GET_CVAR( "world.drawSectors", drawSectors );
	if ( drawSectors != NULL && !drawSectors->b_value )
		return NULL;

	YNCoreWorldSector **visibleSectors = PlCAllocA( world->numSectors, sizeof( YNCoreWorldSector * ) );

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

static YNCoreWorldMesh **GetVisibleSubMeshesForSector( YNCoreWorldSector *sector, const PLGCamera *camera, unsigned int *numMeshes )
{
	PL_GET_CVAR( "world.drawSubMeshes", drawSubMeshes );
	if ( drawSubMeshes != NULL && !drawSubMeshes->b_value )
		return NULL;

	YNCoreWorldMesh **visibleMeshes = PlCAlloc( sector->numStaticObjects, sizeof( YNCoreWorldMesh * ), true );

	// Go through and generate a list of visible meshes within the sector
	for ( unsigned int i = 0; i < sector->numStaticObjects; ++i )
	{
		PLCollisionAABB *bounds = &sector->staticObjects[ i ].mesh->bounds;
		bounds->origin          = sector->staticObjects[ i ].transform.translation;
		if ( !PlgIsBoxInsideView( camera, bounds ) )
			continue;

		visibleMeshes[ *numMeshes ] = sector->mesh;
		numMeshes++;
	}

	// Shrink the array down before passing it back
	visibleMeshes = PlReAlloc( visibleMeshes, sizeof( YNCoreWorldMesh * ) * *numMeshes, true );
	return visibleMeshes;
}

/**
 * This is a little bit silly, but we're considering mirrors as a valid portal too...
 */
bool YnCore_World_IsFacePortal( const YNCoreWorldFace *face )
{
	return ( ( face->flags & WORLD_FACE_FLAG_MIRROR ) || ( face->flags & WORLD_FACE_FLAG_PORTAL ) );
}

YNCoreWorldSector *YnCore_World_GetSectorByNum( YNCoreWorld *world, int sectorNum )
{
	if ( sectorNum < 0 || sectorNum >= world->numSectors )
		return NULL;

	return &world->sectors[ sectorNum ];
}
