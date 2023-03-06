// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

PL_EXTERN_C

#include <yin/core_input.h>

typedef struct NLNode NLNode;

typedef void ( *ClientInputActionCallback )( YNCoreInputState state );

void Client_Input_Initialize( void );
void Client_Input_Shutdown( void );

void Client_Input_SerializeConfig( NLNode *root );
void Client_Input_DeserializeConfig( NLNode *root );

void Client_Input_ClearDevices( void );

void Client_Input_HandleKeyboardEvent( int key, YNCoreInputState keyState );
void Client_Input_HandleMouseButtonEvent( int button, YNCoreInputState buttonState );
void Client_Input_HandleMouseWheelEvent( float x, float y );
void Client_Input_HandleMouseMotionEvent( int x, int y );

void Client_Input_GetMousePosition( int *x, int *y );
void Client_Input_GetMouseDelta( int *x, int *y );

bool Client_Input_GetActionState( );

void Client_Input_BeginFrame( void );
void Client_Input_Tick( void );
void Client_Input_EndFrame( void );

PL_EXTERN_C_END
