// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "engine_private.h"
#include "world.h"

#include "client/renderer/renderer.h"
#include "client/renderer/renderer_material.h"

#define WORLD_VERTEX_ELEMENTS 12// pos, norm, uv, colour

bool World_IsFaceVisible( WorldFace *face, const YRCamera *camera )
{
	// Check the face is actually visible
	face->bounds.origin = pl_vecOrigin3;
	if ( !PlgIsBoxInsideView( camera->internal, &face->bounds ) )
		return false;

	return true;
}

unsigned int *World_ConvertFaceToTriangles( const WorldFace *face, unsigned int *numTriangles )
{
	if ( face->numVertices < 3 )
		return NULL;

	*numTriangles = face->numVertices - 2;
	if ( *numTriangles == 0 )
		return NULL;

	unsigned int *indices = PlMAllocA( sizeof( unsigned int ) * ( *numTriangles * 3 ) );
	unsigned int *index   = indices;
	for ( unsigned int i = 1; i + 1 < face->numVertices; ++i, index += 3 )
	{
		index[ 0 ] = face->vertices[ 0 ];
		index[ 1 ] = face->vertices[ i ];
		index[ 2 ] = face->vertices[ i + 1 ];
	}

	return indices;
}

static void GenerateFaceNormal( const WorldMesh *mesh, WorldFace *face )
{
	for ( unsigned int i = 0; i < face->numVertices; ++i )
		face->normal = PlAddVector3( face->normal, mesh->vertices[ face->vertices[ i ] ].normal );

	face->normal = PlNormalizeVector3( face->normal );
}

static void DeserializeMaterials( NLNode *meshNode, WorldMesh *meshPtr )
{
	NLNode *materialsList = NL_GetChildByName( meshNode, "materials" );
	if ( materialsList == NULL )
	{
		PRINT_WARNING( "No materials for mesh: %s!\n", meshPtr->id );
		return;
	}

	meshPtr->numMaterials = NL_GetNumOfChildren( materialsList );
	meshPtr->materials    = PlCAlloc( meshPtr->numMaterials, sizeof( Material    *), true );
	NLNode *materialNode  = NL_GetFirstChild( materialsList );
	for ( unsigned int i = 0; i < meshPtr->numMaterials; ++i )
	{
		if ( materialNode == NULL )
		{
			PRINT_WARNING( "Hit an invalid material index!\n" );
			meshPtr->numMaterials = i;
			break;
		}

		char materialPath[ PL_SYSTEM_MAX_PATH ];
		NL_GetStr( materialNode, materialPath, sizeof( materialPath ) );
		meshPtr->materials[ i ] = YinCore_Material_Cache( materialPath, CACHE_GROUP_WORLD, true, false );
		materialNode            = NL_GetNextChild( materialNode );
	}
}

static WorldVertex *DeserializeVertices( NLNode *meshNode, unsigned int *numVertices )
{
	NLNode *verticesList = NL_GetChildByName( meshNode, "vertices" );
	if ( verticesList == NULL )
		return NULL;

	unsigned int numChildren = NL_GetNumOfChildren( verticesList );
	float       *data        = PL_NEW_( float, numChildren );
	if ( NL_GetF32Array( verticesList, ( float * ) data, numChildren ) != NL_ERROR_SUCCESS )
	{
		PRINT_WARNING( "Failed to fetch all vertices for mesh!\n" );
		PL_DELETE( data );
		return NULL;
	}

	*numVertices = numChildren / sizeof( WorldVertex );
	return ( WorldVertex * ) data;
}

static void DeserializeFaces( NLNode *meshNode, WorldMesh *worldMesh )
{
	NLNode *facesList = NL_GetChildByName( meshNode, "faces" );
	if ( facesList == NULL )
	{
		PRINT_WARNING( "No faces for mesh: %s!\n", worldMesh->id );
		return;
	}

	unsigned int numFaces = NL_GetNumOfChildren( facesList );
	NLNode      *faceNode = NL_GetFirstChild( facesList );
	for ( unsigned int i = 0; i < numFaces; ++i )
	{
		if ( faceNode == NULL )
		{
			PRINT_WARNING( "Hit an invalid face index!\n" );
			break;
		}

		WorldFace *face = PL_NEW( WorldFace );

		int materialIndex = NL_GetI32ByName( faceNode, "material", -1 );
		if ( materialIndex >= 0 && materialIndex < worldMesh->numMaterials )
			face->material = worldMesh->materials[ materialIndex ];

		face->materialAngle = NL_GetF32ByName( faceNode, "materialAngle", 0.0f );

		NL_DS_DeserializeVector2( NL_GetChildByName( faceNode, "materialOffset" ), &face->materialOffset );
		NL_DS_DeserializeVector2( NL_GetChildByName( faceNode, "materialScale" ), &face->materialScale );

		NLNode *n;
		if ( ( n = NL_GetChildByName( faceNode, "vertices" ) ) != NULL )
		{
			face->numVertices = NL_GetNumOfChildren( n );
			if ( face->numVertices >= WORLD_FACE_MAX_SIDES )
			{
				PRINT_WARNING( "Too many vertices for face: %d!\n", i );
				face->numVertices = WORLD_FACE_MAX_SIDES;
			}

			if ( face->numVertices > 0 )
				NL_GetUI32Array( n, face->vertices, face->numVertices );
		}

		face->flags = NL_GetI32ByName( faceNode, "flags", 0 );

		GenerateFaceNormal( worldMesh, face );

		PlInsertLinkedListNode( worldMesh->faces, face );

		faceNode = NL_GetNextChild( faceNode );
	}
}

/**
 * Deserialise a mesh from the given node.
 */
WorldMesh *WorldDeserialiser_BeginMesh( NLNode *meshNode, WorldMesh *worldMesh )
{
	DeserializeMaterials( meshNode, worldMesh );

	unsigned int numVertices;
	WorldVertex *vertices = DeserializeVertices( meshNode, &numVertices );
	if ( vertices == NULL )
	{
		PRINT_WARNING( "Failed to fetch vertices for mesh: %s\n", worldMesh->id );
		return NULL;
	}
	worldMesh->maxVertices = worldMesh->numVertices = numVertices;
	worldMesh->vertices                             = vertices;

	DeserializeFaces( meshNode, worldMesh );

	return worldMesh;
}

static void GenerateBounds( WorldMesh *mesh )
{
	PLVector3 *coords = PL_NEW_( PLVector3, mesh->numVertices );
	for ( unsigned int i = 0; i < mesh->numVertices; ++i )
	{
		coords[ i ] = mesh->vertices[ i ].position;
	}
	mesh->bounds = PlGenerateAabbFromCoords( coords, mesh->numVertices, true );
	PL_DELETE( coords );

	PLLinkedListNode *faceNode = PlGetFirstNode( mesh->faces );
	while ( faceNode != NULL )
	{
		WorldFace *face = PlGetLinkedListNodeUserData( faceNode );
		coords          = PL_NEW_( PLVector3, face->numVertices );
		for ( unsigned int j = 0; j < face->numVertices; ++j )
			coords[ j ] = mesh->vertices[ face->vertices[ j ] ].position;

		face->bounds = PlGenerateAabbFromCoords( coords, face->numVertices, true );
		PL_DELETE( coords );

		face->origin = face->bounds.absOrigin;

		faceNode = PlGetNextLinkedListNode( faceNode );
	}
}

/**
 * Free the mesh from memory.
 */
void DestroyWorldMesh( WorldMesh *mesh )
{
	PlgDestroyMesh( mesh->drawMesh );
}

WorldMesh *World_Mesh_Create( World *parent )
{
	WorldMesh *mesh = PL_NEW( WorldMesh );
	mesh->faces     = PlCreateLinkedList();

	if ( parent != NULL )
		PlPushBackVectorArrayElement( parent->meshes, mesh );

	MemoryManager_SetupReference( "WorldMesh", MEM_CACHE_WORLD_MESH, &mesh->mem, ( MMReference_CleanupFunction ) DestroyWorldMesh, mesh );
	MemoryManager_AddReference( &mesh->mem );

	return mesh;
}

WorldMesh *World_Mesh_Load( const char *path )
{
	// Check to see if it's cached already
	WorldMesh *worldMesh = MM_GetCachedData( path, MEM_CACHE_WORLD_MESH );
	if ( worldMesh != NULL )
	{
		MemoryManager_AddReference( &worldMesh->mem );
		return worldMesh;
	}

	NLNode *node = NL_LoadFile( path, "worldMesh" );
	if ( node == NULL )
	{
		PRINT_WARNING( "Failed to load world mesh: %s\n", path );
		return NULL;
	}

	worldMesh = World_Mesh_Create( NULL );
	if ( WorldDeserialiser_BeginMesh( node, worldMesh ) == NULL )
	{
		World_Mesh_Release( worldMesh );
		worldMesh = NULL;
	}

	NL_DestroyNode( node );

	// If it loaded fine, be sure we start tracking it
	if ( worldMesh != NULL )
	{
		GenerateBounds( worldMesh );

		worldMesh->drawMesh = PlgCreateMesh( PLG_MESH_TRIANGLES, PLG_DRAW_DYNAMIC, PlGetNumLinkedListNodes( worldMesh->faces ), worldMesh->numVertices );
		if ( worldMesh->drawMesh == NULL )
			PRINT_ERROR( "Failed to create internal mesh for world mesh!\n" );

		// Push all the vertices to the mesh
		for ( unsigned int i = 0; i < worldMesh->numVertices; ++i )
		{
			PlgAddMeshVertex( worldMesh->drawMesh,
			                  worldMesh->vertices[ i ].position,
			                  worldMesh->vertices[ i ].normal,
			                  PlColourF32ToU8( &worldMesh->vertices[ i ].colour ),
			                  worldMesh->vertices[ i ].uv );
		}

		PLLinkedListNode *faceNode = PlGetFirstNode( worldMesh->faces );
		while ( faceNode != NULL )
		{
			WorldFace *face = PlGetLinkedListNodeUserData( faceNode );

			unsigned int  numTriangles;
			unsigned int *indices  = World_ConvertFaceToTriangles( face, &numTriangles );
			unsigned int *curIndex = indices;
			for ( unsigned int k = 0; k < numTriangles; ++k, curIndex += 3 )
				PlgAddMeshTriangle( worldMesh->drawMesh, curIndex[ 0 ], curIndex[ 1 ], curIndex[ 2 ] );

			PL_DELETE( indices );

			g_gfxPerfStats.numFacesDrawn++;

			faceNode = PlGetNextLinkedListNode( faceNode );
		}

		PlgGenerateVertexTangentBasis( worldMesh->drawMesh->vertices, worldMesh->drawMesh->num_verts );
		//PlgGenerateMeshTangentBasis( worldMesh->drawMesh );

		MM_AddToCache( path, MEM_CACHE_WORLD_MESH, worldMesh );
	}

	return worldMesh;
}

void World_Mesh_Release( WorldMesh *worldMesh )
{
	if ( worldMesh == NULL )
		return;

	MemoryManager_ReleaseReference( &worldMesh->mem );
}
