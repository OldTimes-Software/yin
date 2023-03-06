// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "core_private.h"
#include "core_model.h"
#include "node/public/node.h"

static const unsigned int MDL_VERSION = 2;

/**
 * Callback for garbage day.
 */
static void DestroyModel( void *userData )
{
	PLMModel *model = userData;
	assert( model != NULL );

	MDLUserData *additionalData = model->userData;
	if ( additionalData != NULL )
	{
		for ( unsigned int i = 0; i < additionalData->numMaterials; ++i )
			YnCore_Material_Release( additionalData->materials[ i ] );
	}

	PlmDestroyModel( model );
}

static PLGMesh *DeserializeMesh( NLNode *root )
{
	uint32_t numVertices = ( uint32_t ) NL_GetI32ByName( root, "numVertices", 0 );
	if ( numVertices == 0 )
	{
		PRINT_WARNING( "Invalid mesh, no vertices!\n" );
		return NULL;
	}

	NLNode *child;

	child = NL_GetChildByName( root, "triangles" );
	if ( child == NULL )
	{
		PRINT_WARNING( "Invalid mesh, no triangles!\n" );
		return NULL;
	}
	uint32_t  numIndices = NL_GetNumOfChildren( child );
	uint32_t *indices    = PL_NEW_( uint32_t, numIndices );
	NL_GetI32Array( child, ( int32_t * ) indices, numIndices );
	uint32_t numTriangles = numIndices / 3;

	// vertex positions are required
	PLVector3 *positions;
	if ( ( child = NL_GetChildByName( root, "positions" ) ) != NULL )
	{
		positions = PL_NEW_( PLVector3, numVertices );
		NL_GetF32Array( child, ( float * ) positions, numVertices / 3 );
	}
	else
	{
		PL_DELETE( indices );
		PRINT_WARNING( "Invalid mesh, no vertex positions!\n" );
		return NULL;
	}

	// the rest are optional

	PLVector3 *normals = NULL;
	if ( ( child = NL_GetChildByName( root, "normals" ) ) != NULL )
	{
		normals = PL_NEW_( PLVector3, numVertices );
		NL_GetF32Array( child, ( float * ) normals, numVertices / 3 );
	}

	PLVector2 *uvs = NULL;
	if ( ( child = NL_GetChildByName( root, "uvs" ) ) != NULL )
	{
		uvs = PL_NEW_( PLVector2, numVertices );
		NL_GetF32Array( child, ( float * ) uvs, numVertices / 2 );
	}

	PLColourF32 *colours = NULL;
	if ( ( child = NL_GetChildByName( root, "colours" ) ) != NULL )
	{
		colours = PL_NEW_( PLColourF32, numVertices );
		NL_GetF32Array( child, ( float * ) colours, numVertices / 4 );
	}

	PLGMesh *mesh = PlgCreateMesh( PLG_MESH_TRIANGLES, PLG_DRAW_STATIC, numTriangles, numVertices );
	if ( mesh == NULL )
	{
		PRINT_WARNING( "Failed to create mesh: %s\n", PlGetError() );
		return NULL;
	}

	mesh->materialIndex = NL_GetI32ByName( root, "materialIndex", 0 );

	for ( uint32_t i = 0; i < numVertices; ++i )
	{
		PLColour  colour = ( colours == NULL ) ? ( PLColour ){ 255, 255, 255, 255 } : PlColourF32ToU8( &colours[ i ] );
		PLVector3 normal = ( normals == NULL ) ? pl_vecOrigin3 : normals[ i ];
		PLVector2 uv     = ( uvs == NULL ) ? pl_vecOrigin2 : uvs[ i ];
		PlgAddMeshVertex( mesh, positions[ i ], normal, colour, uv );
	}

	for ( uint32_t i = 0; i < numIndices; i += 3 )
		PlgAddMeshTriangle( mesh, indices[ i ], indices[ i + 1 ], indices[ i + 2 ] );

	if ( normals == NULL )
		PlgGenerateMeshNormals( mesh, false );

	PlgGenerateMeshTangentBasis( mesh );
	PlgUploadMesh( mesh );

	return mesh;
}

static PLMModel *DeserializeModel( NLNode *root )
{
	int version = NL_GetI32ByName( root, "version", -1 );
	if ( version == -1 || version > MDL_VERSION )
	{
		PRINT_WARNING( "Invalid model version, %d, expected %u!\n", version, MDL_VERSION );
		return NULL;
	}

	unsigned int numMeshes;
	NLNode      *meshArray = NL_GetChildByName( root, "meshes" );
	if ( meshArray == NULL || ( ( numMeshes = NL_GetNumOfChildren( meshArray ) ) == 0 ) )
	{
		PRINT_WARNING( "No meshes for model!\n" );
		return NULL;
	}

	MDLUserData userData;
	PL_ZERO_( userData );

	// Iterate over all the materials under the root and attempt to load them all in
	NLNode *materialArray = NL_GetChildByName( root, "materials" );
	if ( materialArray == NULL )
	{
		PRINT_WARNING( "No materials for model, using fallback!\n" );
		userData.numMaterials   = 1;
		userData.materials[ 0 ] = YnCore_Material_Cache( "materials/engine/fallback_mesh.mat.n", 0, true, false );
	}
	else
	{
		userData.numMaterials = NL_GetNumOfChildren( materialArray );
		NLNode *n             = NL_GetFirstChild( materialArray );
		for ( unsigned int i = 0; i < userData.numMaterials; ++i )
		{
			assert( n != NULL );
			char materialPath[ PL_SYSTEM_MAX_PATH ];
			if ( NL_GetStr( n, materialPath, sizeof( materialPath ) ) != NL_ERROR_SUCCESS )
			{
				userData.materials[ i ] = YnCore_Material_Cache( "materials/engine/fallback_mesh.mat.n", 0, false, false );
				if ( userData.materials[ i ] == NULL )
				{
					PRINT_ERROR( "Failed to cache fallback material for mesh!\n" );
				}
			}
			else
			{
				userData.materials[ i ] = YnCore_Material_Cache( materialPath, 0, true, false );
			}

			n = NL_GetNextChild( n );
		}
	}

	PLGMesh **meshes   = PL_NEW_( PLGMesh *, numMeshes );
	NLNode   *meshNode = NL_GetFirstChild( meshArray );
	for ( unsigned int i = 0; i < numMeshes; ++i )
	{
		assert( meshNode != NULL );
		meshes[ i ] = DeserializeMesh( meshNode );
		if ( meshes[ i ] == NULL )
			PRINT_ERROR( "Failed to load mesh %u from model!\n" );

		meshNode = NL_GetNextChild( meshNode );
	}

	PLMModel *model;
	if ( NL_GetBoolByName( root, "isAnimated", false ) )
	{
		NLNode  *bonesList = NL_GetChildByName( root, "bones" );
		uint32_t numBones  = NL_GetNumOfChildren( bonesList );

		uint32_t rootBone = ( uint32_t ) NL_GetI32ByName( root, "rootBone", 0 );
		assert( rootBone < numBones );
		if ( rootBone >= numBones )
			PRINT_WARNING( "Invalid root bone (%u), defaulting to 0!\n", rootBone );

		model = PlmCreateSkeletalModel( meshes, numMeshes, NULL, 0, NULL, 0 );
	}
	else
		model = PlmCreateStaticModel( meshes, numMeshes );

	if ( model == NULL )
		PRINT_ERROR( "Failed to create model: %s\n", PlGetError() );

	model->userData                            = PL_NEW( MDLUserData );
	*( ( MDLUserData * ) ( model->userData ) ) = userData;

	return model;
}

PLMModel *Model_Cache( const char *path )
{
	NLNode *root = NL_LoadFile( path, "model" );
	if ( root == NULL )
	{
		PRINT_WARNING( "Invalid model: %s (%s)\n", NL_GetErrorMessage() );
		return NULL;
	}

	PLMModel *model = DeserializeModel( root );
	if ( model == NULL )
		PRINT_WARNING( "Failed to load model, \"%s\"!\n", path );

	NL_DestroyNode( root );

	return model;
}

/**
 * Release a model handle.
 * If it's not tracked by the memory
 * manager then it'll be immediately
 * destroyed.
 */
void Model_Release( PLMModel *model )
{
	MDLUserData *additionalData = model->userData;
	if ( additionalData == NULL )
	{
		PRINT_WARNING( "Destroying model not tracked by memory manager!\n" );
		PlmDestroyModel( model );
		return;
	}

	MemoryManager_ReleaseReference( &additionalData->mem );
}
