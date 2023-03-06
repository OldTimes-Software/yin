// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

/* map everything out to controller-style input
 * even if the user isn't necessarily using a controller
 */
typedef enum YNCoreInputButton
{
	YN_CORE_INPUT_INVALID,

	YN_CORE_INPUT_UP,
	YN_CORE_INPUT_DOWN,
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

	YN_CORE_MAX_BUTTON_INPUTS
} YNCoreInputButton;

typedef enum YNCoreInputMouseButton
{
	CLIENT_INPUT_MOUSE_BUTTON_LEFT,
	CLIENT_INPUT_MOUSE_BUTTON_RIGHT,
	CLIENT_INPUT_MOUSE_BUTTON_MIDDLE,

	YN_CORE_MAX_INPUT_MOUSE_BUTTONS
} YNCoreInputMouseButton;

typedef enum YNCoreInputKey
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

	YN_CORE_MAX_KEY_INPUTS
} YNCoreInputKey;

typedef enum YNCoreInputState
{
	YN_CORE_INPUT_STATE_NONE,     /* key has no state */
	YN_CORE_INPUT_STATE_PRESSED,  /* key has been pressed */
	YN_CORE_INPUT_STATE_DOWN,     /* key is still down */
	YN_CORE_INPUT_STATE_RELEASED, /* key is up */
} YNCoreInputState;

PL_EXTERN_C

typedef enum ClientInputDeviceType
{
	CLIENT_INPUT_DEVICE_NONE,
	CLIENT_INPUT_DEVICE_KEYBOARD,
	CLIENT_INPUT_DEVICE_MOUSE,
	CLIENT_INPUT_DEVICE_TOUCH,
	CLIENT_INPUT_DEVICE_CONTROLLER,
} ClientInputDeviceType;

// Controller API

/**
 * Returns the number of available controllers.
 */
unsigned int YnCore_Input_GetNumControllers( void );

/**
 * Returns the button state for the given slot.
 */
YNCoreInputState YnCore_Input_GetButtonStatus( unsigned int slot, YNCoreInputButton button );

/**
 * Returns the analogue stick state for the given slot.
 */
PLVector2 YnCore_Input_GetStickStatus( unsigned int slot, unsigned int stickNum );

PL_EXTERN_C_END
