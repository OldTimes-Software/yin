// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "core_shell.h"//todo: deprecate this
#include "core_camera.h"
#include "core_editor.h"

PL_EXTERN_C

bool YnCore_Initialize( const char *config );
void YnCore_Shutdown( void );

void YnCore_RenderFrame( YNCoreViewport *viewport );
void YnCore_TickFrame( void );

unsigned int YnCore_GetNumTicks( void );

bool YnCore_IsEngineRunning( void );

void YnCore_HandleKeyboardEvent( int key, unsigned int keyState );
void YnCore_HandleTextEvent( const char *key );
void YnCore_HandleMouseButtonEvent( int button, YNCoreInputState buttonState );
void YnCore_HandleMouseWheelEvent( float x, float y );
void YnCore_HandleMouseMotionEvent( int x, int y );

PL_EXTERN_C_END
