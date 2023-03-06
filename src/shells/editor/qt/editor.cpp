// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <QMessageBox>

#include "editor.h"
#include "editor_mainwindow.h"

extern "C"
{
	YRViewport *OS_Shell_CreateWindow( const char *title, int width, int height, bool fullscreen, uint8_t mode )
	{
		return nullptr;
	}

	void OS_Shell_GetWindowSize( int *width, int *height ) {}
	void YnCore_ShellInterface_DisplayMessageBox( OSMessageType messageType, const char *message, ... )
	{
	}

	OSInputState OS_Shell_GetButtonState( ClientInputButton inputButton ) { return INPUT_STATE_NONE; }
	OSInputState OS_Shell_GetKeyState( int key ) { return INPUT_STATE_NONE; }
	void OS_Shell_GetMousePosition( int *x, int *y ) {}
	void OS_Shell_SetMousePosition( int x, int y ) {}
	void OS_Shell_GrabMouse( bool grab ) {}

	void OS_Shell_Shutdown( void ) {}
}
