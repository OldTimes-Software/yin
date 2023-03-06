// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "core_private.h"
#include "lisp_interface.h"

void Lisp_Interface_Initialize( void )
{
	LspInit();

	// TODO: register user functions

	LspTmpSpace();

	PRINT( "Lisp Interface initialized\n" );
}

void Lisp_Interface_Shutdown( void )
{
	LspUninit();
}

bool Lisp_Interface_CompileScript( const char *path )
{
	char ev[ PL_SYSTEM_MAX_PATH + 16 ];
	snprintf( ev, sizeof( ev ), "(load \"%s\")\n", path );

	char const *cs = ev;
	if ( LspEvalObject( LspCompileObject( cs ) ) == NULL )
	{
		PRINT_WARNING( "Failed to compile \"%s\"\n", path );
		return false;
	}

	LspClearSpace( LspGetTempSpace() );

	return true;
}
