// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "game_private.h"

typedef struct GameMovementComponent
{
	float forwardVelocity;
	float strafeVelocity;

	PLVector3 velocity;

	float maxRunSpeed, maxWalkSpeed;

	YNCoreEntityComponent *inputComponent;
	YNCoreEntityComponent *cameraComponent;
} GameMovementComponent;
#define GAME_MOVEMENT_COMPONENT( A ) ( ( GameMovementComponent * ) ( A ) )

const YNCoreEntityComponentCallbackTable *Game_Component_Movement_GetCallbackTable( void );
