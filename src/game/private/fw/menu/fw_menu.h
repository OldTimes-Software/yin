// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "../fw_game.h"

// base menu impl.
#include "../../game_menu.h"

void FW_Menu_Initialize( void );
void FW_Menu_Tick( void );
void FW_Menu_Draw( const YRViewport *viewport );
bool FW_Menu_HandleInput( void );
