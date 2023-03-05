// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "engine_private.h"
#include "world.h"
#include "entity/entity.h"
#include "client/renderer/renderer_material.h"

static void DeserializeIdentifierTag( NLNode *node, char *dest )
{
	dest[ WORLD_PROP_TAG_LENGTH ] = '\0';
	const char *id                = NL_GetStrByName( node, "id", NULL );
	if ( id == NULL )
	{
		PlGenerateUniqueIdentifier( dest, WORLD_PROP_TAG_LENGTH - 1 );
		return;
	}

	strncpy( dest, id, WORLD_PROP_TAG_LENGTH - 1 );
}

static void DeserialiseSector( World *world, NLNode *sectorNode, WorldSector *sectorPtr )
{
	DeserializeIdentifierTag( sectorNode, sectorPtr->id );

	unsigned int numMeshes = PlGetNumVectorArrayElements( world->meshes );
	int          meshIndex = NL_GetI32ByName( sectorNode, "mesh", -1 );
	if ( meshIndex >= 0 && meshIndex < numMeshes )
	{
		sectorPtr->mesh = ( WorldMesh * ) PlGetVectorArrayElementAt( world->meshes, meshIndex );
	}
	else
	{
		PRINT_WARNING( "Sector without valid body!\n" );
	}

	NL_DS_DeserializeVector3( NL_GetChildByName( sectorNode, "boundsMin" ), &sectorPtr->bounds.mins );
	NL_DS_DeserializeVector3( NL_GetChildByName( sectorNode, "boundsMax" ), &sectorPtr->bounds.maxs );

	NLNode *staticObjectList = NL_GetChildByName( sectorNode, "staticObjects" );
	if ( staticObjectList != NULL )
	{
		sectorPtr->numStaticObjects = NL_GetNumOfChildren( staticObjectList );
		sectorPtr->staticObjects    = PlCAlloc( sectorPtr->numStaticObjects, sizeof( WorldObject ), true );
		NLNode *c                   = NL_GetFirstChild( staticObjectList );
		for ( unsigned int i = 0; i < sectorPtr->numStaticObjects; ++i )
		{
			if ( c == NULL )
			{
				PRINT_WARNING( "Hit an invalid object index: %d\n", i );
				sectorPtr->numStaticObjects = i;
				break;
			}

			meshIndex = NL_GetI32ByName( sectorNode, "mesh", -1 );
			if ( meshIndex >= 0 && meshIndex < numMeshes )
			{
				sectorPtr->staticObjects[ i ].mesh = ( WorldMesh * ) PlGetVectorArrayElementAt( world->meshes, meshIndex );
			}
			else
			{
				PRINT_WARNING( "Invalid mesh index encountered for static object!\n" );
			}

			NL_DS_DeserializeVector3( NL_GetChildByName( c, "translation" ), &sectorPtr->staticObjects[ i ].transform.translation );
			NL_DS_DeserializeVector3( NL_GetChildByName( c, "scale" ), &sectorPtr->staticObjects[ i ].transform.scale );
			NL_DS_DeserializeVector4( NL_GetChildByName( c, "rotation" ), ( PLVector4 * ) &sectorPtr->staticObjects[ i ].transform.rotation );

			c = NL_GetNextChild( c );
		}
	}
}

static void DeserialiseEntities( World *world, NLNode *root )
{
	if ( root == NULL )
	{
		PRINT( "No entities in world, skipping.\n" );
		return;
	}

	unsigned int entityNum = 0;
	NLNode      *child     = NL_GetFirstChild( root );
	while ( child != NULL )
	{
		const char *templateName = NL_GetStrByName( child, "templateName", NULL );
		if ( templateName != NULL )
		{
			const EntityPrefab *entityTemplate = YinCore_EntityManager_GetPrefabByName( templateName );
			if ( entityTemplate != NULL )
			{
				WorldEntity *worldEntity    = PL_NEW( WorldEntity );
				worldEntity->entityTemplate = entityTemplate;

				NLNode *properties = NL_GetChildByName( child, "properties" );
				if ( properties != NULL )
				{
					worldEntity->properties = NL_CopyNode( properties );
				}

				PlInsertLinkedListNode( world->entities, worldEntity );
			}
			else
			{
				PRINT_WARNING( "Failed to find entity template \"%s\"!\n", templateName );
			}
		}
		else
		{
			PRINT_WARNING( "No template name provided for entity %u!\n", entityNum );
		}

		child = NL_GetNextChild( child );
		entityNum++;
	}
}

World *WorldDeserialiser_Begin( NLNode *root, World *out )
{
	int version = NL_GetI32ByName( root, "version", -1 );
	if ( version == -1 )
	{
		PRINT_WARNING( "Failed to find world version!\n" );
		return NULL;
	}
	else if ( version > WORLD_VERSION )
	{
		PRINT_WARNING( "Unsupported world version! (%d > %d)\n", version, WORLD_VERSION );
		return NULL;
	}

	NLNode *propertyList = NL_GetChildByName( root, "properties" );
	if ( propertyList != NULL )
	{
		out->globalProperties = NL_CopyNode( propertyList );

		/* set some of the global defaults */
		World_SetupGlobalDefaults( out );

		NL_DS_DeserializeColourF32( NL_GetChildByName( out->globalProperties, "ambience" ), &out->ambience );
		NL_DS_DeserializeColourF32( NL_GetChildByName( out->globalProperties, "sunColour" ), &out->sunColour );
		NL_DS_DeserializeVector3( NL_GetChildByName( out->globalProperties, "sunPosition" ), &out->sunPosition );
		NL_DS_DeserializeColourF32( NL_GetChildByName( out->globalProperties, "clearColour" ), &out->clearColour );

		NL_DS_DeserializeColourF32( NL_GetChildByName( out->globalProperties, "fogColour" ), &out->fogColour );
		out->fogFar  = NL_GetF32ByName( out->globalProperties, "fogFar", 11.0f );
		out->fogNear = NL_GetF32ByName( out->globalProperties, "fogNear", 32.0f );

		NLNode *childProperty = NL_GetChildByName( out->globalProperties, "skyMaterials" );
		if ( childProperty != NULL )
		{
			out->numSkyMaterials = NL_GetNumOfChildren( childProperty );
			if ( out->numSkyMaterials > MAX_SKY_LAYERS )
			{
				PRINT_WARNING( "Only a maximum of %d sky layers are supported!\n", MAX_SKY_LAYERS );
				out->numSkyMaterials = MAX_SKY_LAYERS;
			}

			unsigned int i          = 0;
			NLNode      *childIndex = NL_GetFirstChild( childProperty );
			while ( childIndex != NULL )
			{
				char buf[ PL_SYSTEM_MAX_PATH ];
				NL_GetStr( childIndex, buf, sizeof( buf ) );
				out->skyMaterials[ i++ ] = YinCore_Material_Cache( buf, CACHE_GROUP_WORLD, true, false );

				childIndex = NL_GetNextChild( childIndex );
			}
		}
	}

	DeserialiseEntities( out, NL_GetChildByName( root, "entities" ) );

	NLNode *meshList = NL_GetChildByName( root, "meshes" );
	if ( meshList != NULL )
	{
		unsigned int numEntries = NL_GetNumOfChildren( meshList );
		out->meshes             = PlCreateVectorArray( numEntries );
		NLNode *c               = NL_GetFirstChild( meshList );
		for ( unsigned int i = 0; i < numEntries; ++i )
		{
			if ( c == NULL )
			{
				PRINT_WARNING( "Hit an invalid mesh index: %d\n", i );
				break;
			}

			PLPath path;
			NL_GetStr( c, path, sizeof( path ) );

			WorldMesh *mesh = World_Mesh_Load( path );
			if ( mesh == NULL )
				continue;

			PlPushBackVectorArrayElement( out->meshes, mesh );
		}

		// Check if we need to downsize the meshes list...
		PlShrinkVectorArray( out->meshes );
	}

	NLNode *sectorList = NL_GetChildByName( root, "sectors" );
	if ( sectorList != NULL )
	{
		out->numSectors = NL_GetNumOfChildren( sectorList );
		out->sectors    = PlCAlloc( out->numSectors, sizeof( WorldSector ), true );
		NLNode *c       = NL_GetFirstChild( sectorList );
		for ( unsigned int i = 0; i < out->numSectors; ++i )
		{
			if ( c == NULL )
			{
				PRINT_WARNING( "Hit an invalid sector Index: %d\n", i );
				out->numSectors = i;
				break;
			}

			DeserialiseSector( out, c, &out->sectors[ i ] );
		}
	}
	else
		PRINT_WARNING( "No sectors specified for world!\n" );

	return out;
}
