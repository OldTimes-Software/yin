// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "game_component_movement.h"

static void HandleMouseLook( GameMovementComponent *movementComponent )
{
	PL_GET_CVAR( "input.mlook", mouseLook );
	if ( mouseLook == NULL || !mouseLook->b_value )
		return;

	int mx, my;
	OS_Shell_GetMousePosition( &mx, &my );//TODO: should use Client_Input_GetMouseDelta ...

	//TODO
}

static void Tick( EntityComponent *self )
{
	if ( OS_Shell_GetButtonState( INPUT_A ) )
		GAME_MOVEMENT_COMPONENT( self )->velocity.y += 10.0f;

	HandleMouseLook( GAME_MOVEMENT_COMPONENT( self ) );

	static const float gain = 0.25f;

	if ( OS_Shell_GetButtonState( INPUT_UP ) || OS_Shell_GetKeyState( 'w' ) )
		GAME_MOVEMENT_COMPONENT( self )->forwardVelocity += gain;
	else if ( OS_Shell_GetButtonState( INPUT_DOWN ) || OS_Shell_GetKeyState( 's' ) )
		GAME_MOVEMENT_COMPONENT( self )->forwardVelocity -= gain;
	else if ( GAME_MOVEMENT_COMPONENT( self )->forwardVelocity != 0.0f )
	{
		GAME_MOVEMENT_COMPONENT( self )->forwardVelocity = GAME_MOVEMENT_COMPONENT( self )->forwardVelocity > 0 ? GAME_MOVEMENT_COMPONENT( self )->forwardVelocity - gain : GAME_MOVEMENT_COMPONENT( self )->forwardVelocity + gain;
		if ( GAME_MOVEMENT_COMPONENT( self )->forwardVelocity < 0.1f && GAME_MOVEMENT_COMPONENT( self )->forwardVelocity > -0.1f )
			GAME_MOVEMENT_COMPONENT( self )->forwardVelocity = 0.0f;
	}

	// strafing
	if ( OS_Shell_GetKeyState( 'a' ) )
		GAME_MOVEMENT_COMPONENT( self )->strafeVelocity += gain;
	else if ( OS_Shell_GetKeyState( 'd' ) )
		GAME_MOVEMENT_COMPONENT( self )->strafeVelocity -= gain;
	else if ( GAME_MOVEMENT_COMPONENT( self )->strafeVelocity != 0.0f )
	{
		GAME_MOVEMENT_COMPONENT( self )->strafeVelocity = GAME_MOVEMENT_COMPONENT( self )->strafeVelocity > 0 ? GAME_MOVEMENT_COMPONENT( self )->strafeVelocity - gain : GAME_MOVEMENT_COMPONENT( self )->strafeVelocity + gain;
		if ( GAME_MOVEMENT_COMPONENT( self )->strafeVelocity < 0.1f && GAME_MOVEMENT_COMPONENT( self )->strafeVelocity > -0.1f )
			GAME_MOVEMENT_COMPONENT( self )->strafeVelocity = 0.0f;
	}

	// clamp the velocity as necessary
	float maxVelocity = OS_Shell_GetButtonState( INPUT_LEFT_STICK ) || OS_Shell_GetKeyState( KEY_LEFT_SHIFT ) ? GAME_MOVEMENT_COMPONENT( self )->maxRunSpeed : GAME_MOVEMENT_COMPONENT( self )->maxWalkSpeed;
	GAME_MOVEMENT_COMPONENT( self )->forwardVelocity = PlClamp( -maxVelocity, GAME_MOVEMENT_COMPONENT( self )->forwardVelocity, maxVelocity );
	GAME_MOVEMENT_COMPONENT( self )->strafeVelocity = PlClamp( -maxVelocity, GAME_MOVEMENT_COMPONENT( self )->strafeVelocity, maxVelocity );

#if 0
	PLVector3 left;
	PlAnglesAxes( GAME_MOVEMENT_COMPONENT( self )->viewAngles, &left, NULL, &self->forward );
	GAME_MOVEMENT_COMPONENT( self )->velocity = PlAddVector3( GAME_MOVEMENT_COMPONENT( self )->velocity,
	                               PlAddVector3(
	                                       PlScaleVector3F( GAME_MOVEMENT_COMPONENT( self )->forward, GAME_MOVEMENT_COMPONENT( self )->forwardVelocity ),
	                                       PlScaleVector3F( left, GAME_MOVEMENT_COMPONENT( self )->strafeVelocity ) ) );
#endif
}

const EntityComponentCallbackTable *Game_Component_Movement_GetCallbackTable( void )
{
	static EntityComponentCallbackTable callbackTable;
	PL_ZERO_( callbackTable );

	callbackTable.tickFunction = Tick;

	return &callbackTable;
}
