// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_physics.h>
#include <plcore/pl_array_vector.h>

#include "client/renderer/renderer_scenegraph.h"
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

enum YNCoreWorldFaceFlag
{
	PL_BITFLAG( WORLD_FACE_FLAG_PORTAL, 0U ), /* reflect portal */
	PL_BITFLAG( WORLD_FACE_FLAG_MIRROR, 1U ), /* reflect back */
	PL_BITFLAG( WORLD_FACE_FLAG_SKIP, 2U ),   /* skip face */
};

typedef enum YNCoreWorldObjectCollisionType
{
	WORLD_OBJECT_COLLISION_POLY,
	WORLD_OBJECT_COLLISION_SPHERE,
	WORLD_OBJECT_COLLISION_AABB,
} YNCoreWorldObjectCollisionType;

#define WORLD_FACE_MAX_SIDES 32

typedef struct YNCoreWorldSector YNCoreWorldSector;
typedef struct YNCoreWorldFace YNCoreWorldFace;
typedef struct YNCoreWorldMesh YNCoreWorldMesh;

typedef struct YNCoreWorldFace
{
	PLVector3 normal;
	PLVector3 origin;

	struct YNCoreMaterial *material;
	// todo: reduce the below to transform matrix???
	float materialAngle;
	PLVector2 materialOffset;
	PLVector2 materialScale;

	unsigned int vertices[ WORLD_FACE_MAX_SIDES ];
	uint8_t numVertices;

	uint8_t flags; /* portal, mirror, skip etc. */

	YNCoreWorldMesh *parentMesh;
	YNCoreWorldSector *parentSector;

	// if it's a portal
	bool isPortalClosed;              // if true, we can't see through the portal
	YNCoreWorldSector *targetSector;  // the sector this portal connects to
	YNCoreWorldFace *targetSectorFace;// the 'door' on the other side

	PLCollisionAABB bounds;
} YNCoreWorldFace;

typedef struct YNCoreWorldVertex
{
	PLVector3 position;
	PLVector3 normal;
	PLVector2 uv;
	PLColourF32 colour;
} YNCoreWorldVertex;

typedef struct YNCoreWorldMesh
{
	char id[ WORLD_PROP_TAG_LENGTH ];

	struct YNCoreMaterial **materials;
	unsigned int numMaterials;

	YNCoreWorldVertex *vertices;
	unsigned int numVertices;
	unsigned int maxVertices;

	PLLinkedList *faces;

	PLCollisionAABB bounds;

	PLGMesh *drawMesh; /* what actually gets rendered */

	PLLinkedListNode *node;

	YNCoreMemoryReference mem;
} YNCoreWorldMesh;

typedef struct YNCoreWorldObject
{
	YNCoreWorldMesh *mesh; /* pointer to mesh in worldMeshes list */

	SGTransform transform;

	YNCoreWorldObjectCollisionType collisionType;
	union
	{
		const YNCoreWorldMesh *collisionMesh;
		const PLCollisionAABB *collisionBounds;
	} collisionPtr;
} YNCoreWorldObject;

typedef struct YNCoreWorldSector
{
	char id[ WORLD_PROP_TAG_LENGTH ];

	YNCoreWorldMesh *mesh;

	YNCoreWorldObject *staticObjects;
	unsigned int numStaticObjects;

	PLLinkedList *actors;// Actors currently in this sector
	PLLinkedList *lights;// Lights in this sector

	PLCollisionAABB bounds;
} YNCoreWorldSector;

#define YN_CORE_MAX_SKY_LAYERS 4

typedef struct YNCoreWorld
{
	PLPath path;

	PLVectorArray *meshes;

	PLLinkedList *entities;

	YNCoreWorldSector *sectors;
	unsigned int numSectors;

	PLColourF32 ambience;
	PLColourF32 sunColour;
	PLVector3 sunPosition;

	PLColourF32 clearColour;

	PLColourF32 fogColour;
	float fogNear;
	float fogFar;

	struct YNCoreMaterial *skyMaterials[ YN_CORE_MAX_SKY_LAYERS ];
	unsigned int numSkyMaterials;

	/* additional generic properties */
	struct YNNodeBranch *globalProperties;

	uint64_t lastSaveTime;
	bool isDirty;
} YNCoreWorld;

typedef struct YNCoreWorldEntity
{
	const YNCoreEntityPrefab *entityTemplate;
	YNNodeBranch *properties;
} YNCoreWorldEntity;

#include <yin/core_world.h>

void YnCore_WorldSerialiser_Begin( const YNCoreWorld *world, YNNodeBranch *root );
YNCoreWorld *YnCore_WorldDeserialiser_Begin( YNNodeBranch *root, YNCoreWorld *out );

YNCoreWorldMesh *YnCore_WorldDeserialiser_BeginMesh( YNNodeBranch *root, YNCoreWorldMesh *worldMesh );

PLLinkedList *YnCore_World_GetLights( const YNCoreWorld *world );
PLLinkedList *YnCore_World_GetSectorLights( const YNCoreWorldSector *sector );

void YnCore_World_SpawnEntities( YNCoreWorld *world );

bool YnCore_World_IsFaceVisible( YNCoreWorldFace *face, const YNCoreCamera *camera );
unsigned int *YnCore_World_ConvertFaceToTriangles( const YNCoreWorldFace *face, unsigned int *numTriangles );
bool YnCore_World_IsFacePortal( const YNCoreWorldFace *face );

YNCoreWorldSector *YnCore_World_GetSectorByNum( YNCoreWorld *world, int sectorNum );
