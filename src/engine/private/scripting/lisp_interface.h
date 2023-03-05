// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "lisp.h" /* our c interface */

void Lisp_Interface_Initialize( void );
void Lisp_Interface_Shutdown( void );

bool Lisp_Interface_CompileScript( const char *path );
