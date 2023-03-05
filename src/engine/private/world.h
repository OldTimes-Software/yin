// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_physics.h>
#include <plcore/pl_array_vector.h>

#include "client/renderer/scenegraph.h"
#include "entity/entity.h"

#define WORLD_PROP_TAG_LENGTH   64
#define WORLD_PROP_VALUE_LENGTH 256

#if 0 /* original values, used for prototype */
#	define WORLD_DEFAULT_AMBIENCE    PL_COLOURF32( 0.4f, 0.4f, 0.4f, 1.0f )
#	define WORLD_DEFAULT_CLEARCOLOUR PL_COLOURF32( 0.0f, 0.0f, 0.0f, 1.0f )
#	define WORLD_DEFAULT_SKY         "materials/sky/cloudlayer00.mat.n"
#	define WORLD_DEFAULT_SUNPOSITION PLVector3( 0.5f, -1.0f, 0.5f )
#	define WORLD_DEFAULT_SUNCOLOUR   PL_COLOURF32( 1.0f, 1.0f, 1.0f, 1.25f )
#else
#	define WORLD_DEFAULT_AMBIENCE    PL_COLOURF32( 0.0f, 0.0f, 0.0f, 1.0f )
#	define WORLD_DEFAULT_CLEARCOLOUR PL_COLOURF32( 0.0f, 0.0f, 0.0f, 1.0f )
#	define WORLD_DEFAULT_SKY         "materials/sky/cloudlayer00.mat.n"
#	define WORLD_DEFAULT_SUNPOSITION PLVector3( 0.5f, -1.0f, 0.5f )
#	define WORLD_DEFAULT_SUNCOLOUR   PL_COLOURF32( 0.0f, 0.0f, 0.0f, 0.0f )
#endif

enum WorldFaceFlag
{
	PL_BITFLAG( WORLD_FACE_FLAG_PORTAL, 0U ), /* reflect portal */
	PL_BITFLAG( WORLD_FACE_FLAG_MIRROR, 1U ), /* reflect back */
	PL_BITFLAG( WORLD_FACE_FLAG_SKIP, 2U ),   /* skip face */
};

typedef enum WorldObjectCollisionType
{
	WORLD_OBJECT_COLLISION_POLY,
	WORLD_OBJECT_COLLISION_SPHERE,
	WORLD_OBJECT_COLLISION_AABB,
} WorldObjectCollisionType;

#define WORLD_FACE_MAX_SIDES 32

typedef struct WorldSector WorldSector;
typedef struct WorldFace WorldFace;
typedef struct WorldMesh WorldMesh;

typedef struct WorldFace
{
	PLVector3 normal;
	PLVector3 origin;

	struct Material *material;
	// todo: reduce the below to transform matrix???
	float materialAngle;
	PLVector2 materialOffset;
	PLVector2 materialScale;

	unsigned int vertices[ WORLD_FACE_MAX_SIDES ];
	uint8_t numVertices;

	uint8_t flags; /* portal, mirror, skip etc. */

	WorldMesh *parentMesh;
	WorldSector *parentSector;

	// if it's a portal
	bool isPortalClosed;        // if true, we can't see through the portal
	WorldSector *targetSector;  // the sector this portal connects to
	WorldFace *targetSectorFace;// the 'door' on the other side

	PLCollisionAABB bounds;
} WorldFace;

typedef struct WorldVertex
{
	PLVector3 position;
	PLVector3 normal;
	PLVector2 uv;
	PLColourF32 colour;
} WorldVertex;

typedef struct WorldMesh
{
	char id[ WORLD_PROP_TAG_LENGTH ];

	struct Material **materials;
	unsigned int numMaterials;

	WorldVertex *vertices;
	unsigned int numVertices;
	unsigned int maxVertices;

	PLLinkedList *faces;

	PLCollisionAABB bounds;

	PLGMesh *drawMesh; /* what actually gets rendered */

	PLLinkedListNode *node;

	MMReference mem;
} WorldMesh;

typedef struct WorldObject
{
	WorldMesh *mesh; /* pointer to mesh in worldMeshes list */

	SGTransform transform;

	WorldObjectCollisionType collisionType;
	union
	{
		const WorldMesh *collisionMesh;
		const PLCollisionAABB *collisionBounds;
	} collisionPtr;
} WorldObject;

typedef struct WorldSector
{
	char id[ WORLD_PROP_TAG_LENGTH ];

	WorldMesh *mesh;

	WorldObject *staticObjects;
	unsigned int numStaticObjects;

	PLLinkedList *actors;// Actors currently in this sector
	PLLinkedList *lights;// Lights in this sector

	PLCollisionAABB bounds;
} WorldSector;

#define MAX_SKY_LAYERS 4

typedef struct World
{
	PLPath path;

	PLVectorArray *meshes;

	PLLinkedList *entities;

	WorldSector *sectors;
	unsigned int numSectors;

	PLColourF32 ambience;
	PLColourF32 sunColour;
	PLVector3 sunPosition;

	PLColourF32 clearColour;

	PLColourF32 fogColour;
	float fogNear;
	float fogFar;

	struct Material *skyMaterials[ MAX_SKY_LAYERS ];
	unsigned int numSkyMaterials;

	/* additional generic properties */
	struct NLNode *globalProperties;

	uint64_t lastSaveTime;
	bool isDirty;
} World;

typedef struct WorldEntity
{
	const EntityPrefab *entityTemplate;
	NLNode *properties;
} WorldEntity;

#include "engine/public/engine_public_world.h"

void WorldSerialiser_Begin( const World *world, NLNode *root );
World *WorldDeserialiser_Begin( NLNode *root, World *out );

WorldMesh *WorldDeserialiser_BeginMesh( NLNode *root, WorldMesh *worldMesh );

PLLinkedList *World_GetLights( const World *world );
PLLinkedList *World_GetSectorLights( const WorldSector *sector );

void World_SpawnEntities( World *world );

bool World_IsFaceVisible( WorldFace *face, const YRCamera *camera );
unsigned int *World_ConvertFaceToTriangles( const WorldFace *face, unsigned int *numTriangles );
bool World_IsFacePortal( const WorldFace *face );

WorldSector *World_GetSectorByNum( World *world, int sectorNum );
