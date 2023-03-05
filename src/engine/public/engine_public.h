// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_math.h>

#include "yin_os.h"//todo: deprecate this

#include "engine_public_camera.h"
#include "engine_public_editor.h"

typedef enum EngineMode
{
	ENGINE_MODE_DEFAULT,
	ENGINE_MODE_EDITOR,

	ENGINE_MAX_MODES
} EngineMode;

PL_EXTERN_C

bool Engine_Initialize( const char *config );
void Engine_Shutdown( void );

void YinCore_RenderFrame( YRViewport *viewport );
void Engine_TickFrame( void );

unsigned int Engine_GetNumTicks( void );

bool YinCore_IsEngineRunning( void );

void Engine_HandleKeyboardEvent( int key, unsigned int keyState );
void Engine_HandleTextEvent( const char *key );
void Engine_HandleMouseButtonEvent( int button, OSInputState buttonState );
void Engine_HandleMouseWheelEvent( float x, float y );
void YinCore_HandleMouseMotionEvent( int x, int y );

PL_EXTERN_C_END
