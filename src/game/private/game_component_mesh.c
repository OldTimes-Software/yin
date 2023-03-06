/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plmodel/plm.h>

#include "game_private.h"

typedef struct ECMesh
{
	PLMModel *model;
} ECMesh;

static void EntityComponent_Mesh_Spawn( YNCoreEntityComponent *self )
{
	ECMesh *mesh = PlMAllocA( sizeof( ECMesh ) );
	self->userData = mesh;
}

static void EntityComponent_Mesh_Destroy( YNCoreEntityComponent *self )
{
	ECMesh *mesh = self->userData;

	PlmDestroyModel( mesh->model );

	PlFree( mesh );
}

const YNCoreEntityComponentCallbackTable *EntityComponent_Mesh_GetCallbackTable( void )
{
	static YNCoreEntityComponentCallbackTable callbackTable;
	memset( &callbackTable, 0, sizeof( YNCoreEntityComponentCallbackTable ) );

	callbackTable.spawnFunction = EntityComponent_Mesh_Spawn;

	return &callbackTable;
}
