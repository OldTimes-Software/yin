// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

typedef enum YRCameraMode
{
	YR_CAMERA_MODE_PERSPECTIVE,
	YR_CAMERA_MODE_TOP,
	YR_CAMERA_MODE_LEFT,
	YR_CAMERA_MODE_FRONT,

	YR_CAMERA_MAX_MODES
} YRCameraMode;

typedef enum YRCameraDrawMode
{
	YR_CAMERA_DRAW_MODE_WIREFRAME,
	YR_CAMERA_DRAW_MODE_SOLID,
	YR_CAMERA_DRAW_MODE_TEXTURED,

	YR_CAMERA_MAX_DRAW_MODES
} YRCameraDrawMode;

typedef struct YRCamera YRCamera;

PL_EXTERN_C

YRCamera *YR_Camera_Create( const char *tag, const PLVector3 *position, const PLVector3 *angles );
void      YR_Camera_Destroy( YRCamera *camera );
void      YR_Camera_SetPosition( YRCamera *camera, const PLVector3 *position );
void      YR_Camera_SetAngles( YRCamera *camera, const PLVector3 *angles );

YRCamera *YR_GetActiveCamera( void );
void      YR_MakeCameraActive( YRCamera *camera );

PL_EXTERN_C_END
