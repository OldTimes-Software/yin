// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "core_private.h"

#include "client/client_input.h"

/****************************************
 * CONSOLE OUTPUT BUFFER
 ****************************************/

static ConsoleOutput conOutputBuffer;

ConsoleOutput *Console_GetOutput( void )
{
	return &conOutputBuffer;
}

static void ClearOutputBuffer( void )
{
	conOutputBuffer.numLines = 0;
}

static void ClearConsoleCommand( unsigned int argc, char **argv )
{
	( void ) ( argc );
	( void ) ( argv );
	ClearOutputBuffer();
}

static void OutputCallback( int level, const char *message, PLColour colour )
{
	size_t l = strlen( message );
	if ( l >= CONSOLE_BUFFER_MAX_LENGTH )
	{
		PRINT_WARNING( "Attempting to push message to console with an unexpected length!\n" );
		l = CONSOLE_BUFFER_MAX_LENGTH - 2;
	}

	if ( conOutputBuffer.numLines >= CONSOLE_BUFFER_MAX_LINES )
	{
#define CON_JUMP 256
		memmove( conOutputBuffer.lines, &conOutputBuffer.lines[ CON_JUMP ], CONSOLE_BUFFER_MAX_LINES - CON_JUMP );
		conOutputBuffer.numLines -= CON_JUMP;
	}

	strncpy( conOutputBuffer.lines[ conOutputBuffer.numLines ].buffer, message, l );
	conOutputBuffer.lines[ conOutputBuffer.numLines ].buffer[ l ] = '\0';

	conOutputBuffer.lines[ conOutputBuffer.numLines ].colour = colour;
	conOutputBuffer.numLines++;

	YnCore_ShellInterface_PushMessage( level, message, &colour );
}

/* CONSOLE COMMANDS */

#define CMD_CALLBACK( NAME ) static void Cmd_##NAME( unsigned int argc, char **argv )

CMD_CALLBACK( Quit )
{
	( void ) ( argc );
	( void ) ( argv );
    YnCore_Shutdown();
}

/**
 * Pipes a command to either the shell or command.
 */
CMD_CALLBACK( OSCommand )
{
	if ( argc == 1 )
	{
		PRINT_WARNING( "Usage: oscmd echo \"Hello World!\"\n" );
		return;
	}

	if ( system( argv[ 1 ] ) == -1 )
		PRINT_WARNING( "Failed to issue command, an error occurred!\n" );
}

CMD_CALLBACK( Version )
{
	( void ) ( argc );
	( void ) ( argv );
	PRINT( "Version: v" ENGINE_VERSION_STR " [" GIT_BRANCH "." GIT_COMMIT_COUNT "]\n" );
}

/*------------------------------------------------------------------*/

#include "node/public/node.h"
#include "core_filesystem.h"

static void SaveUserConfig( void );
static void LoadUserConfig( void )
{
	NLNode *root = NL_LoadFile( FileSystem_GetUserConfigLocation(), "config" );
	if ( root == NULL )
	{
		PRINT( "No existing user config, generating default.\n" );
		SaveUserConfig();
		return;
	}

	/* now iterate through the list and update all our children */
	NLNode *child = NL_GetFirstChild( root );
	while ( child != NULL )
	{
		const char *cvarName = NL_GetName( child );
		char        cvarValue[ PL_SYSTEM_MAX_PATH ];
		if ( NL_GetStr( child, cvarValue, sizeof( cvarValue ) ) == NL_ERROR_SUCCESS )
			PlSetConsoleVariableByName( cvarName, cvarValue );
		else
			PRINT_WARNING( "Failed to fetch value: %s\n", cvarName );

		child = NL_GetNextChild( child );
	}

	Client_Input_DeserializeConfig( root );

	PRINT( "User config loaded.\n" );
}

static void SaveUserConfig( void )
{
	char path[ PL_SYSTEM_MAX_PATH ];
	snprintf( path, sizeof( path ), "%s", FileSystem_GetUserConfigLocation() );
	PRINT_DEBUG( "Saving user config: \"%s\"\n", path );

	PLConsoleVariable **cvars;
	size_t              numVars;
	PlGetConsoleVariables( &cvars, &numVars );

	NLNode *root = NL_PushBackObj( NULL, "config" );
	for ( unsigned int i = 0; i < numVars; ++i )
	{
		/* don't bother storing it if it matches the default */
		if ( strcmp( cvars[ i ]->value, cvars[ i ]->default_value ) == 0 )
			continue;

		switch ( cvars[ i ]->type )
		{
			case PL_VAR_F32:
				NL_PushBackF32( root, cvars[ i ]->name, cvars[ i ]->f_value );
				break;
			case PL_VAR_I32:
				NL_PushBackI32( root, cvars[ i ]->name, cvars[ i ]->i_value );
				break;
			case PL_VAR_BOOL:
				NL_PushBackBool( root, cvars[ i ]->name, cvars[ i ]->b_value );
				break;
			default:
				NL_PushBackStr( root, cvars[ i ]->name, cvars[ i ]->s_value );
				break;
		}
	}

	Client_Input_SerializeConfig( root );

	NL_WriteFile( path, root, NL_FILE_UTF8 );
	NL_DestroyNode( root );

	PRINT( "User config saved.\n" );
}

void YnCore_RegisterConsoleCommands( bool isDedicated )
{
	PlRegisterConsoleCommand( "Quit", "Shutdown any existing server and terminate the application.", 0, Cmd_Quit );
	PlRegisterConsoleCommand( "Exit", "Shutdown any existing server and terminate the application.", 0, Cmd_Quit );
	PlRegisterConsoleCommand( "OSCmd", "Pipes the given command to the host platform.", -1, Cmd_OSCommand );
	PlRegisterConsoleCommand( "Version", "Prints out the current engine version.", 0, Cmd_Version );
	PlRegisterConsoleCommand( "Clear", "Clear the console buffer.", 0, ClearConsoleCommand );
	PlRegisterConsoleCommand( "CLS", "Clear the console buffer.", 0, ClearConsoleCommand );

	if ( !isDedicated )
	{
		Client_Console_RegisterConsoleCommands();
	}
}

void YnCore_RegisterConsoleVariables( bool isDedicated )
{
	// server
	PlRegisterConsoleVariable( "server.name", "Name to use for the server.", "unnamed", PL_VAR_STRING, NULL, NULL, false );
	PlRegisterConsoleVariable( "server.password", "Password to access server functions.", "", PL_VAR_STRING, NULL, NULL, false );

	// Client variables
	if ( !isDedicated )
		Client_Console_RegisterConsoleVariables();
}

static int logLevels[ YINENGINE_LOG_LEVELS ];

int Console_GetLogLevel( ConsoleLogLevel level )
{
	return logLevels[ level ];
}

void Console_Print( ConsoleLogLevel level, const char *message, ... )
{
	va_list args;
	va_start( args, message );

	int length = pl_vscprintf( message, args ) + 1;
	if ( length <= 0 )
		return;

	char *buf = PL_NEW_( char, length );
	vsnprintf( buf, length, message, args );

	va_end( args );

	PlLogMessage( logLevels[ level ], buf );

	PL_DELETE( buf );
}

/**
 * Set the console up.
 */
void YnCore_InitializeConsole( void )
{
	PlSetConsoleOutputCallback( OutputCallback );

	logLevels[ YINENGINE_LOG_ERROR ]       = PlAddLogLevel( "yin/error", PL_COLOUR_RED, true );
	logLevels[ YINENGINE_LOG_WARNING ]     = PlAddLogLevel( "yin/warning", PL_COLOUR_YELLOW, true );
	logLevels[ YINENGINE_LOG_INFORMATION ] = PlAddLogLevel( "yin", PL_COLOUR_WHITE, true );

	logLevels[ YINENGINE_LOG_CLIENT_ERROR ]       = PlAddLogLevel( "yin/client/error", PL_COLOUR_RED, true );
	logLevels[ YINENGINE_LOG_CLIENT_WARNING ]     = PlAddLogLevel( "yin/client/warning", PL_COLOUR_YELLOW, true );
	logLevels[ YINENGINE_LOG_CLIENT_INFORMATION ] = PlAddLogLevel( "yin/client", PL_COLOUR_WHITE, true );

	logLevels[ YINENGINE_LOG_SERVER_ERROR ]       = PlAddLogLevel( "yin/server/error", PL_COLOUR_RED, true );
	logLevels[ YINENGINE_LOG_SERVER_WARNING ]     = PlAddLogLevel( "yin/server/warning", PL_COLOUR_YELLOW, true );
	logLevels[ YINENGINE_LOG_SERVER_INFORMATION ] = PlAddLogLevel( "yin/server", PL_COLOUR_WHITE, true );
}

void YnCore_ShutdownConsole( void )
{
	ClearOutputBuffer();
	SaveUserConfig();
}
