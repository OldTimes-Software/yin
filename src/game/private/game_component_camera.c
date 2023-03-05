// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "game_private.h"
#include "game_component_transform.h"

typedef struct GameComponentCamera
{
	YRCamera        *camera;
	bool             isActive;
	EntityComponent *transform;
} GameComponentCamera;
#define GCCAMERA( SELF ) ENTITY_COMPONENT_CAST( ( SELF ), GameComponentCamera )

ENTITY_COMPONENT_BEGIN_PROPERTIES()
ENTITY_COMPONENT_PROPERTY( GameComponentCamera, isActive, "Indicates if the camera should be active or not.", CMN_DATATYPE_BOOL )
ENTITY_COMPONENT_END_PROPERTIES()

static void Spawn( EntityComponent *self )
{
	self->userData = PL_NEW( GameComponentCamera );

	const PLVector3 *position, *angles;

	GCCAMERA( self )->transform = YinCore_Entity_GetComponentByName( self->entity, "transform" );
	if ( GCCAMERA( self )->transform != NULL )
	{
		position = &ECTRANSFORM( GCCAMERA( self )->transform )->translation;
		angles   = &ECTRANSFORM( GCCAMERA( self )->transform )->angles;
	}
	else
	{
		position = &pl_vecOrigin3;
		angles   = &pl_vecOrigin3;
	}

	GCCAMERA( self )->camera = YR_Camera_Create( "dummy", position, angles );
}

static void Destroy( EntityComponent *self )
{
	YR_Camera_Destroy( GCCAMERA( self )->camera );

	PL_DELETE( GCCAMERA( self ) );
}

static void Tick( EntityComponent *self )
{
	// if there's no transform component, try checking again...
	if ( GCCAMERA( self )->transform == NULL )
		GCCAMERA( self )->transform = YinCore_Entity_GetComponentByName( self->entity, "transform" );
	if ( GCCAMERA( self )->transform == NULL )
		return;

	YR_Camera_SetPosition( GCCAMERA( self )->camera, &ECTRANSFORM( GCCAMERA( self )->transform )->translation );
	YR_Camera_SetAngles( GCCAMERA( self )->camera, &ECTRANSFORM( GCCAMERA( self )->transform )->angles );

	if ( GCCAMERA( self )->isActive )
		YR_MakeCameraActive( GCCAMERA( self )->camera );
}

const EntityComponentCallbackTable *Game_Component_Camera_GetCallbackTable( void )
{
	static EntityComponentCallbackTable callbackTable;
	PL_ZERO_( callbackTable );
	callbackTable.spawnFunction   = Spawn;
	callbackTable.destroyFunction = Destroy;
	callbackTable.tickFunction    = Tick;

	ENTITY_HOOK_PROPERTIES( callbackTable );

	return &callbackTable;
}
