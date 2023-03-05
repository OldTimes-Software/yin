/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plmodel/plm.h>

#include "game_private.h"

typedef struct ECMesh
{
	PLMModel *model;
} ECMesh;

static void EntityComponent_Mesh_Spawn( EntityComponent *self )
{
	ECMesh *mesh = PlMAllocA( sizeof( ECMesh ) );
	self->userData = mesh;
}

static void EntityComponent_Mesh_Destroy( EntityComponent *self )
{
	ECMesh *mesh = self->userData;

	PlmDestroyModel( mesh->model );

	PlFree( mesh );
}

const EntityComponentCallbackTable *EntityComponent_Mesh_GetCallbackTable( void )
{
	static EntityComponentCallbackTable callbackTable;
	memset( &callbackTable, 0, sizeof( EntityComponentCallbackTable ) );

	callbackTable.spawnFunction = EntityComponent_Mesh_Spawn;

	return &callbackTable;
}
