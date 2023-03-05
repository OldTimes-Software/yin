/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include "gui_public.h"

void      Client_GUI_Initialize( void );
void      Client_GUI_Shutdown( void );
void      Client_GUI_Draw( const YRViewport *viewport );
void      Client_GUI_Tick( void );
void      Client_GUI_Resize( int w, int h );
GUIPanel *Client_GUI_GetRootPanel( void );
