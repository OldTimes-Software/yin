// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_linkedlist.h>

#include "node/public/node.h"

PL_EXTERN_C

typedef char EntityClassName[ 64 ];
typedef char EntityName[ 64 ];

typedef struct Entity Entity;
typedef struct EntityPrefab EntityPrefab;
typedef struct EntityComponentBase EntityComponentBase;
typedef struct EntityComponent
{
	const EntityComponentBase *base;
	Entity *entity;
	struct PLLinkedListNode *listNode;
	void *userData;
} EntityComponent;

#define ENTITY_COMPONENT_CAST( SELF, TYPE ) ( ( TYPE * ) ( SELF )->userData )

typedef void ( *ECSpawnFunction )( EntityComponent *self );
typedef void ( *ECTickFunction )( EntityComponent *self );
typedef void ( *ECDrawFunction )( EntityComponent *self );
typedef void ( *ECDestroyFunction )( EntityComponent *self );
typedef NLNode *( *ECSerializeFunction )( EntityComponent *self, NLNode *root );
typedef NLNode *( *ECDeserializeFunction )( EntityComponent *self, NLNode *root );

typedef struct EntityComponentCallbackTable
{
	ECSpawnFunction spawnFunction;
	ECTickFunction tickFunction;
	ECDrawFunction drawFunction;
	ECDestroyFunction destroyFunction;
	ECSerializeFunction serializeFunction;
	ECDeserializeFunction deserializeFunction;

	const struct EditorField *editorFields;
	unsigned int numEditorFields;
} EntityComponentCallbackTable;

void YinCore_EntityManager_Initialize( void );
void YinCore_EntityManager_Shutdown( void );
void YinCore_EntityManager_Tick( void );
void YinCore_EntityManager_Draw( YRCamera *camera, WorldSector *sector );
void YinCore_EntityManager_Save( NLNode *root );
void YinCore_EntityManager_Restore( NLNode *root );

// Prefabs
void YinCore_EntityManager_RegisterEntityPrefab( const char *path );
void YinCore_EntityManager_RegisterEntityPrefabs( void );
const EntityPrefab *YinCore_EntityManager_GetPrefabByName( const char *name );

Entity *YinCore_EntityManager_CreateEntity( void );
Entity *YinCore_EntityManager_CreateEntityFromPrefab( const char *name );
void YinCore_EntityManager_DestroyEntity( Entity *entity );

/**
 * Returns the total number of active entities.
 */
unsigned int YinCore_EntityManager_GetNumOfEntities( void );

bool YinCore_EntityManager_RegisterComponent( const char *name, const EntityComponentCallbackTable *callbackTable );
const EntityComponentBase *YinCore_EntityManager_GetComponentBaseByName( const char *name );
EntityComponent *YinCore_EntityManager_AddComponentToEntity( Entity *entity, const char *name );

/**
 * Returns a list of properties that can be modified for the component.
 */
const struct EditorField *YinCore_EntityComponent_GetEditableProperties( const EntityComponent *entityComponent, unsigned int *num );

/****************************************
 * ENTITY
 ****************************************/

NLNode *YinCore_Entity_Serialize( Entity *self, NLNode *root );
Entity *YinCore_Entity_Deserialize( NLNode *root );

EntityComponent *YinCore_Entity_GetComponentByName( Entity *self, const char *name );
EntityComponent *YinCore_Entity_AttachComponentByName( Entity *self, const char *name );
void YinCore_Entity_RemoveComponent( Entity *self, EntityComponent *component );
void YinCore_Entity_RemoveAllComponents( Entity *self );

PL_EXTERN_C_END
