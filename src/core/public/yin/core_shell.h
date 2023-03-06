// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl.h>

#include "core_renderer.h"
#include "core_input.h"

/* ======================================================================
 * OS/SHELL INTERFACE
 * TODO: should this really remain part of core?
 * ====================================================================*/

PL_EXTERN_C

#define YN_CORE_TICK_RATE ( 1000 / 60 ) /* ms */

typedef enum YNCoreMessageType
{
	YN_CORE_MESSAGE_ERROR,
	YN_CORE_MESSAGE_WARNING,
	YN_CORE_MESSAGE_INFO,
} YNCoreMessageType;

enum
{
	YN_CORE_GRAPHICS_SOFTWARE,
	YN_CORE_GRAPHICS_OPENGL,
	YN_CORE_GRAPHICS_VULKAN,
	YN_CORE_GRAPHICS_OTHER,

	YN_CORE_MAX_GRAPHICS_MODES
};

////////////////////////////////////////////////////////////////////
// Window Management
YNCoreViewport *YnCore_ShellInterface_CreateWindow( const char *title, int width, int height, bool fullscreen, uint8_t mode );
bool YnCore_ShellInterface_SetWindowSize( int *width, int *height );
void YnCore_ShellInterface_GetWindowSize( int *width, int *height );
void YnCore_ShellInterface_DisplayMessageBox( YNCoreMessageType messageType, const char *message, ... );

////////////////////////////////////////////////////////////////////
// Low Level Input
YNCoreInputState YnCore_ShellInterface_GetButtonState( YNCoreInputButton inputButton );
YNCoreInputState YnCore_ShellInterface_GetKeyState( int key );
void YnCore_ShellInterface_GetMousePosition( int *x, int *y );
void YnCore_ShellInterface_SetMousePosition( int x, int y );
void YnCore_ShellInterface_GrabMouse( bool grab );
void YnCore_ShellInterface_PushMessage( int level, const char *msg, const PLColour *colour );
void YnCore_ShellInterface_Shutdown( void );

PL_EXTERN_C_END
