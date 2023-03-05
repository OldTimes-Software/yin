// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_linkedlist.h>

#include "engine_private.h"
#include "renderer.h"
#include "postprocessing/postprocessing.h"
#include "legacy/actor.h"

/* Camera management fun! */

static PLLinkedList *cameras;

static YRCamera *activeCamera = NULL;

YRCamera *YR_GetActiveCamera( void )
{
	return activeCamera;
}

void YR_MakeCameraActive( YRCamera *camera )
{
	activeCamera = camera;
}

/****************************************
 ****************************************/

YRCamera *YR_Camera_Create( const char *tag, const PLVector3 *position, const PLVector3 *angles )
{
	YRCamera *camera = PL_NEW( YRCamera );

	camera->mode = YR_CAMERA_MODE_PERSPECTIVE;

	camera->internal = PlgCreateCamera();
	if ( camera->internal == NULL )
		PRINT_ERROR( "Failed to create camera!\nPL: %s\n", PlGetError() );

	if ( tag != NULL )
		strncpy( camera->tag, tag, sizeof( camera->tag ) - 1 );

	camera->internal->fov      = 75.0f;
	camera->internal->far      = 1000000.0f;
	camera->internal->position = *position;
	camera->internal->angles   = *angles;

	if ( cameras == NULL )
	{
		cameras = PlCreateLinkedList();
		if ( cameras == NULL )
		{
			PRINT_ERROR( "Failed to create cameras list: %s\n", PlGetError() );
		}
	}

	camera->node = PlInsertLinkedListNode( cameras, camera );

	return camera;
}

/**
 * Destroy the given camera. Use this instead
 * of calling PlgDestroyCamera directly, as it
 * will free up user data.
 */
void YR_Camera_Destroy( YRCamera *camera )
{
	if ( camera == NULL )
	{
		return;
	}

	PlgDestroyCamera( camera->internal );

	PlDestroyLinkedListNode( camera->node );

	// be sure the global camera gets unset if we're destroying it
	if ( camera == activeCamera )
	{
		activeCamera = NULL;
	}

	PL_DELETE( camera );

	if ( PlGetNumLinkedListNodes( cameras ) == 0 )
	{
		PlDestroyLinkedList( cameras );
		cameras = NULL;
	}
}

void YR_Camera_SetPosition( YRCamera *camera, const PLVector3 *position )
{
	camera->internal->position = *position;
}

void YR_Camera_SetAngles( YRCamera *camera, const PLVector3 *angles )
{
	camera->internal->angles = *angles;
}

void YR_DrawScene( YRCamera *camera, const YRViewport *viewport );
void YR_DrawPerspective( YRCamera *camera, const YRViewport *viewport )
{
	if ( camera == NULL )
	{
		camera = YR_GetActiveCamera();
		if ( camera == NULL )
		{
			return;
		}
	}

	PL_GET_CVAR( "r.fov", fov );
	if ( fov != NULL )
	{
		PlgSetCameraFieldOfView( camera->internal, fov->f_value );
	}
	PL_GET_CVAR( "r.near", near );
	if ( near != NULL )
	{
		camera->internal->near = near->f_value;
	}
	PL_GET_CVAR( "r.far", far );
	if ( far != NULL )
	{
		camera->internal->far = far->f_value;
	}

#if 0//TODO: revisit...
	PL_GET_CVAR( "r.superSampling", superSampling );
	if ( superSampling != NULL && superSampling->i_value > 1 )
	{
		camera->internal->viewport.w *= superSampling->i_value;
		camera->internal->viewport.h *= superSampling->i_value;
	}
#endif

	static const float minHeight = 256.0f;
	static const float maxHeight = 1024.0f;

	PLVector3 angles;
	PLVector3 position;
	float     speed;

	/* if we have a parent, follow them */
	Actor *parent = camera->parentActor;
	if ( parent != NULL )
	{
		angles.x = parent->viewPitch;
		angles.y = -parent->angles.y + 90.0f;//-Act_GetAngle( camera->parentActor ) + 90.0f;
		angles.z = 0.0f;

		position   = parent->position;
		position.y = Act_GetViewOffset( camera->parentActor );
	}
	else
	{
		angles   = pl_vecOrigin3;
		position = pl_vecOrigin3;
	}

	switch ( camera->mode )
	{
		default: break;
		case YR_CAMERA_MODE_PERSPECTIVE:
			camera->internal->angles   = angles;
			camera->internal->position = position;
			break;
		case YR_CAMERA_MODE_TOP:
		{
			if ( camera->parentActor != NULL )
			{
				speed = PlVector3Length( camera->parentActor->velocity ) / 16.0f;
				if ( speed > 1.0f )
					speed = 1.0f;
			}
			else
				speed = 0.0f;

			camera->internal->angles.x = -75.0f;
			camera->internal->position = position;
			camera->internal->position.x -= 150.0f;
			camera->internal->position.y += minHeight + PlCosineInterpolate( minHeight, maxHeight, speed );
			break;
		}
	}

	PlgSetupCamera( camera->internal );

	// Draw the scene into a buffer
	YR_DrawScene( camera, viewport );
}
