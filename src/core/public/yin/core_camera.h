// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

typedef enum YNCoreCameraMode
{
	YN_CORE_CAMERA_MODE_PERSPECTIVE,
	YN_CORE_CAMERA_MODE_TOP,
	YN_CORE_CAMERA_MODE_LEFT,
	YN_CORE_CAMERA_MODE_FRONT,

	YN_CORE_CAMERA_MAX_MODES
} YNCoreCameraMode;

typedef enum YNCoreCameraDrawMode
{
	YN_CORE_CAMERA_DRAW_MODE_WIREFRAME,
	YN_CORE_CAMERA_DRAW_MODE_SOLID,
	YN_CORE_CAMERA_DRAW_MODE_TEXTURED,

	YN_CORE_CAMERA_MAX_DRAW_MODES
} YNCoreCameraDrawMode;

typedef struct YNCoreCamera YNCoreCamera;

PL_EXTERN_C

YNCoreCamera *YnCore_Camera_Create( const char *tag, const PLVector3 *position, const PLVector3 *angles );
void YnCore_Camera_Destroy( YNCoreCamera *camera );
void YnCore_Camera_SetPosition( YNCoreCamera *camera, const PLVector3 *position );
void YnCore_Camera_SetAngles( YNCoreCamera *camera, const PLVector3 *angles );

YNCoreCamera *YnCore_GetActiveCamera( void );
void YnCore_MakeCameraActive( YNCoreCamera *camera );

PL_EXTERN_C_END
