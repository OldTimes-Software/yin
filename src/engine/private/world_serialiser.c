/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include "engine_private.h"
#include "world.h"

#include "client/renderer/renderer_material.h"

static void SerialiseFace( const WorldFace *face, const WorldMesh *mesh, NLNode *root, const char *name )
{
	NLNode *node = NL_PushBackObj( root, name );
	NL_PushBackStr( node, "material", Eng_Ren_Material_GetPath( face->material ) );
	NL_PushBackF32( node, "materialAngle", face->materialAngle );
	NL_DS_SerializeVector2( node, "materialOffset", &face->materialOffset );
	NL_DS_SerializeVector2( node, "materialScale", &face->materialScale );
	NL_PushBackI8( node, "flags", face->flags );
	NL_DS_SerializeVector3( node, "normal", &face->normal );
}

static void SerialiseFaces( const WorldMesh *mesh, NLNode *root )
{
#if 0
	NLNode *faceListNode = NL_PushBackObjArray( root, "faces" );
	for ( unsigned int i = 0; i < mesh->numFaces; ++i )
	{
		SerialiseFace( &mesh->faces[ i ], mesh, faceListNode, NULL );
	}
#endif
}

static void SerialiseMesh( const WorldMesh *mesh, NLNode *root, const char *name )
{
	NLNode *meshNode = NL_PushBackObj( root, name );
	if ( *mesh->id != '\0' )
		NL_PushBackStr( meshNode, "id", mesh->id );

	NL_DS_SerializeCollisionAABB( meshNode, "bounds", &mesh->bounds );

	SerialiseFaces( mesh, meshNode );
}

static void SerialiseMeshes( const World *world, NLNode *root )
{
	NLNode *meshListNode = NL_PushBackObjArray( root, "meshes" );

	unsigned int numMeshes = PlGetNumVectorArrayElements( world->meshes );
	for ( unsigned int i = 0; i < numMeshes; ++i )
	{
		SerialiseMesh( ( WorldMesh * ) PlGetVectorArrayElementAt( world->meshes, i ), meshListNode, NULL );
	}
}

static void SerialiseSectors( const World *world, NLNode *root )
{
	NLNode *sectorListNode = NL_PushBackObjArray( root, "sectors" );
	for ( unsigned int i = 0; i < world->numSectors; ++i )
	{
		NLNode *sectorNode = NL_PushBackObj( sectorListNode, NULL );

		if ( *world->sectors[ i ].id != '\0' )
			NL_PushBackStr( sectorNode, "id", world->sectors[ i ].id );

		if ( world->sectors[ i ].mesh != NULL && *world->sectors[ i ].mesh->id != '\0' )
			NL_PushBackStr( sectorNode, "meshId", world->sectors[ i ].mesh->id );

		NL_DS_SerializeCollisionAABB( sectorNode, "bounds", &world->sectors[ i ].bounds );
	}
}

void WorldSerialiser_Begin( const World *world, NLNode *root )
{
	NL_PushBackI32( root, "version", WORLD_VERSION );
	NL_PushBackNode( root, world->globalProperties );

	SerialiseMeshes( world, root );
	SerialiseSectors( world, root );
}
