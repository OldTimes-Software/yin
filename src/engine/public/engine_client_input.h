// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

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
unsigned int Eng_Cli_Input_GetNumControllers( void );

/**
 * Returns the button state for the given slot.
 */
OSInputState YinCore_Input_GetButtonStatus( unsigned int slot, ClientInputButton button );

/**
 * Returns the analogue stick state for the given slot.
 */
PLVector2 YinCore_Input_GetStickStatus( unsigned int slot, unsigned int stickNum );

PL_EXTERN_C_END
