/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl.h>

#include "engine_public_renderer.h"

/* ======================================================================
 * OS/LAUNCHER INTERFACE
 * ====================================================================*/

PL_EXTERN_C

#define TICK_RATE ( 1000 / 60 ) /* ms */

typedef void *OSDisplay;

/* map everything out to controller-style input
 * even if the user isn't necessarily using a controller
 */
typedef enum ClientInputButton
{
	INPUT_INVALID,

	INPUT_UP,
	INPUT_DOWN,
	INPUT_LEFT,
	INPUT_RIGHT,

	INPUT_LEFT_STICK,
	INPUT_RIGHT_STICK,

	INPUT_START,
	INPUT_BACK,

	INPUT_A,
	INPUT_B,
	INPUT_X,
	INPUT_Y,

	INPUT_LB,
	INPUT_LT,
	INPUT_RB,
	INPUT_RT,

	MAX_BUTTON_INPUTS
} ClientInputButton;

typedef enum ClientInputMouseButton
{
	CLIENT_INPUT_MOUSE_BUTTON_LEFT,
	CLIENT_INPUT_MOUSE_BUTTON_RIGHT,
	CLIENT_INPUT_MOUSE_BUTTON_MIDDLE,

	MAX_CLIENT_INPUT_MOUSE_BUTTONS
} ClientInputMouseButton;

typedef enum ClientInputKey
{
	KEY_INVALID = -1,

	KEY_BACKSPACE = 8,
	KEY_TAB       = 9,
	KEY_ENTER     = 13,

	KEY_CAPSLOCK = 128,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,

	KEY_PRINTSCREEN,
	KEY_SCROLLLOCK,
	KEY_PAUSE,
	KEY_INSERT,
	KEY_HOME,
	KEY_PAGEUP,
	KEY_PAGEDOWN,
	KEY_DELETE,
	KEY_END,

	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,

	KEY_LEFT_CTRL,
	KEY_RIGHT_CTRL,
	KEY_LEFT_SHIFT,
	KEY_RIGHT_SHIFT,
	KEY_LEFT_ALT,
	KEY_RIGHT_ALT,

	MAX_KEY_INPUTS
} ClientInputKey;

typedef enum OSInputState
{
	INPUT_STATE_NONE,     /* key has no state */
	INPUT_STATE_PRESSED,  /* key has been pressed */
	INPUT_STATE_DOWN,     /* key is still down */
	INPUT_STATE_RELEASED, /* key is up */
} OSInputState;

typedef enum OSMessageType
{
	OS_MESSAGE_ERROR,
	OS_MESSAGE_WARNING,
	OS_MESSAGE_INFO,
} OSMessageType;

typedef struct OSViewport
{
	int x, y;
	int w, h;
} OSViewport;

enum
{
	OS_GRAPHICS_SOFTWARE,
	OS_GRAPHICS_OPENGL,
	OS_GRAPHICS_VULKAN,
	OS_GRAPHICS_OTHER,

	OS_MAX_GRAPHICS_MODES
};

////////////////////////////////////////////////////////////////////
// Window Management
bool OS_IsWindowActive( void );
YRViewport *OS_Shell_CreateWindow( const char *title, int width, int height, bool fullscreen, uint8_t mode );
void OS_DestroyWindow( void );
void OS_SetWindowTitle( const char *title );
bool OS_SetWindowSize( int *width, int *height );
void OS_Shell_GetWindowSize( int *width, int *height );

void OS_Shell_DisplayMessageBox( OSMessageType messageType, const char *message, ... );

////////////////////////////////////////////////////////////////////
// Low Level Input
OSInputState OS_Shell_GetButtonState( ClientInputButton inputButton );
OSInputState OS_Shell_GetKeyState( int key );
void OS_Shell_GetMousePosition( int *x, int *y );
void OS_Shell_SetMousePosition( int x, int y );
void OS_Shell_GrabMouse( bool grab );
bool OS_GetGrabState( void );
void OS_Shell_PushMessage( int level, const char *msg, const PLColour *colour );

void OS_Shell_Shutdown( void );

const char *OS_GetError( void );

PL_EXTERN_C_END
