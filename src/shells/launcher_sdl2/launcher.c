// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <SDL2/SDL.h>

#ifdef _WIN32
#	include <crtdbg.h>
#endif

#include <yin/core.h>
#include <yin/core_renderer.h>
#include <yin/node.h>

#include "common.h"
#include "launcher.h"

static YNNodeBranch *shellConfig;

void YnCore_ShellInterface_PushMessage( int level, const char *msg, const PLColour *colour )
{
}

/****************************************
 * WINDOW MANAGEMENT
 ****************************************/

static SDL_Window *sdlWindow      = NULL;
static SDL_GLContext sdlGLContext = NULL;

static YNCoreViewport *windowViewport = NULL;

static int drawW, drawH;

void YnCore_ShellInterface_DisplayMessageBox( YNCoreMessageType messageType, const char *message, ... )
{
	const char *title;
	SDL_MessageBoxFlags flags;
	switch ( messageType )
	{
		case YN_CORE_MESSAGE_ERROR:
			title = "Error";
			flags = SDL_MESSAGEBOX_ERROR;
			break;
		case YN_CORE_MESSAGE_WARNING:
			title = "Warning";
			flags = SDL_MESSAGEBOX_WARNING;
			break;
		default:
		case YN_CORE_MESSAGE_INFO:
			title = "Info";
			flags = SDL_MESSAGEBOX_INFORMATION;
			break;
	}

	va_list args;
	va_start( args, message );

	int l     = pl_vscprintf( message, args );
	char *buf = PlMAllocA( l + 1 );

	vsnprintf( buf, l, message, args );

	va_end( args );

	PlLogWFunction( launcherLog, "%s", buf );

	SDL_ShowSimpleMessageBox( flags, title, buf, NULL );

	PL_DELETE( buf );
}

static bool IsWindowActive( void )
{
	assert( sdlWindow != NULL );
	uint32_t flags = SDL_GetWindowFlags( sdlWindow );
	return ( !( flags & SDL_WINDOW_HIDDEN ) && ( flags & SDL_WINDOW_INPUT_FOCUS ) );
}

YNCoreViewport *YnCore_ShellInterface_CreateWindow( const char *title, int width, int height, bool fullscreen, uint8_t mode )
{
	int flags = 0;
#if !NDEBUG
	flags |= SDL_WINDOW_RESIZABLE;
#endif
	if ( fullscreen )
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	switch ( mode )
	{
		default:
			PrintWarn( "Unknown graphics mode (%d)!\n", mode );
			break;
		case YN_CORE_GRAPHICS_OPENGL:
			flags |= SDL_WINDOW_OPENGL;
			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
			SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
			SDL_GL_SetAttribute( SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1 );
			break;
		case YN_CORE_GRAPHICS_VULKAN:
			flags |= SDL_WINDOW_VULKAN;
			break;
	}

	sdlWindow = SDL_CreateWindow(
	        title,
	        SDL_WINDOWPOS_CENTERED,
	        SDL_WINDOWPOS_CENTERED,
	        width, height,
	        flags );
	if ( sdlWindow == NULL )
	{
		PrintWarn( "Failed to create window!\nSDL: %s\n", SDL_GetError() );
		return false;
	}

#if 0
#	if !NDEBUG// for debug builds, throw it onto a second display if available
	if ( SDL_GetNumVideoDisplays() > 1 )
	{
		SDL_SetWindowPosition( sdlWindow, SDL_WINDOWPOS_CENTERED_DISPLAY( 1 ), SDL_WINDOWPOS_CENTERED_DISPLAY( 1 ) );
		SDL_MaximizeWindow( sdlWindow );
	}
#	endif
#endif

	if ( mode == YN_CORE_GRAPHICS_OPENGL )
	{
		sdlGLContext = SDL_GL_CreateContext( sdlWindow );
		if ( sdlGLContext == NULL )
		{
			SDL_DestroyWindow( sdlWindow );
			PrintWarn( "Failed to create OpenGL context!\nSDL: %s\n", SDL_GetError() );
			return false;
		}

		SDL_GL_MakeCurrent( sdlWindow, sdlGLContext );
		SDL_GL_GetDrawableSize( sdlWindow, &drawW, &drawH );
	}

	return YnCore_Viewport_Create( 0, 0, width, height, sdlWindow );
}

static void DestroyWindow( void )
{
	if ( sdlGLContext != NULL )
		SDL_GL_DeleteContext( sdlGLContext );

	if ( sdlWindow != NULL )
		SDL_DestroyWindow( sdlWindow );
}

bool YnCore_ShellInterface_SetWindowSize( int *width, int *height )
{
	if ( sdlWindow == NULL )
	{
		*width  = 0;
		*height = 0;
		return false;
	}

	SDL_SetWindowSize( sdlWindow, *width, *height );

	int nW, nH;
	SDL_GetWindowSize( sdlWindow, &nW, &nH );

	if ( *width == nW && *height == nH )
		return true;

	*width  = nW;
	*height = nH;
	return false;
}

void YnCore_ShellInterface_GetWindowSize( int *width, int *height )
{
	SDL_GetWindowSize( sdlWindow, width, height );
}

/****************************************
 * INPUT MANAGEMENT
 ****************************************/

static YNCoreInputState buttonStates[ YN_CORE_MAX_BUTTON_INPUTS ];
YNCoreInputState YnCore_ShellInterface_GetButtonState( YNCoreInputButton inputButton )
{
	if ( inputButton >= YN_CORE_MAX_BUTTON_INPUTS )
		return YN_CORE_INPUT_STATE_NONE;

	return buttonStates[ inputButton ];
}

static YNCoreInputState keyStates[ YN_CORE_MAX_KEY_INPUTS ];
YNCoreInputState YnCore_ShellInterface_GetKeyState( int key )
{
	if ( key >= YN_CORE_MAX_KEY_INPUTS )
		return YN_CORE_INPUT_STATE_NONE;

	return keyStates[ key ];
}

void YnCore_ShellInterface_GetMousePosition( int *x, int *y )
{
	SDL_GetMouseState( x, y );
}

void YnCore_ShellInterface_SetMousePosition( int x, int y )
{
	SDL_WarpMouseInWindow( sdlWindow, x, y );
}

static bool grabState = false;

void YnCore_ShellInterface_GrabMouse( bool grab )
{
	SDL_SetWindowGrab( sdlWindow, grab );
	SDL_SetRelativeMouseMode( grab );

	SDL_ShowCursor( !grab );
}

static int Sys_TranslateSDLKeyInput( int key )
{
	if ( key < 128 )
		return key;

	switch ( key )
	{
		default:
			break;
		case SDLK_CAPSLOCK:
			return KEY_CAPSLOCK;
		case SDLK_F1:
			return KEY_F1;
		case SDLK_F2:
			return KEY_F2;
		case SDLK_F3:
			return KEY_F3;
		case SDLK_F4:
			return KEY_F4;
		case SDLK_F5:
			return KEY_F5;
		case SDLK_F6:
			return KEY_F6;
		case SDLK_F7:
			return KEY_F7;
		case SDLK_F8:
			return KEY_F8;
		case SDLK_F9:
			return KEY_F9;
		case SDLK_F10:
			return KEY_F10;
		case SDLK_F11:
			return KEY_F11;
		case SDLK_F12:
			return KEY_F12;
		case SDLK_PRINTSCREEN:
			return KEY_PRINTSCREEN;
		case SDLK_SCROLLLOCK:
			return KEY_SCROLLLOCK;
		case SDLK_PAUSE:
			return KEY_PAUSE;
		case SDLK_INSERT:
			return KEY_INSERT;
		case SDLK_HOME:
			return KEY_HOME;
		case SDLK_PAGEUP:
			return KEY_PAGEUP;
		case SDLK_PAGEDOWN:
			return KEY_PAGEDOWN;
		case SDLK_DELETE:
			return KEY_DELETE;
		case SDLK_END:
			return KEY_END;
		case SDLK_KP_TAB:
		case SDLK_TAB:
			return KEY_TAB;
		case SDLK_KP_ENTER:
			return KEY_ENTER;
		case SDLK_UP:
			return KEY_UP;
		case SDLK_DOWN:
			return KEY_DOWN;
		case SDLK_LEFT:
			return KEY_LEFT;
		case SDLK_RIGHT:
			return KEY_RIGHT;
		case SDLK_LCTRL:
			return KEY_LEFT_CTRL;
		case SDLK_RCTRL:
			return KEY_RIGHT_CTRL;
		case SDLK_LSHIFT:
			return KEY_LEFT_SHIFT;
		case SDLK_RSHIFT:
			return KEY_RIGHT_SHIFT;
		case SDLK_LALT:
			return KEY_LEFT_ALT;
		case SDLK_RALT:
			return KEY_RIGHT_ALT;

			/* temp temp temp */
		case SDLK_ESCAPE:
			YnCore_Shutdown();
			break;
	}

	return KEY_INVALID;
}

/****************************************
 * TIMER MANAGEMENT
 ****************************************/

static SDL_TimerID sdlTimer = 0;
static unsigned int OS_TimerCallback( unsigned int interval, void *param )
{
#if 1
	SDL_UserEvent userEvent;
	userEvent.type = SDL_USEREVENT;
	userEvent.code = 0;

	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user = userEvent;

	SDL_PushEvent( &event );
#else
	Yin_TickFrame();
#endif

	return interval;
}

/****************************************
 * INITIALIZATION
 ****************************************/

void YnCore_ShellInterface_Shutdown( void )
{
	Common_WriteConfig( shellConfig, "shell" );

	if ( sdlTimer != 0 )
		SDL_RemoveTimer( sdlTimer );

	exit( EXIT_SUCCESS );
}

int launcherLog;

static bool InitializeDisplay( void )
{
	PlgInitializeGraphics();

	// attempt to fetch the driver directly from the executable location if possible
	PLPath exePath;
	if ( PlGetExecutableDirectory( exePath, sizeof( exePath ) ) != NULL )
	{
		size_t size      = strlen( exePath ) + PL_SYSTEM_MAX_PATH + 1;
		char *driverPath = PL_NEW_( char, size );
		snprintf( driverPath, size, "local://%s", exePath );
		PlgScanForDrivers( driverPath );
		PL_DELETE( driverPath );
	}
	else
	{
		PrintWarn( "Failed to get executable location: %s\n", PlGetError() );
		PlgScanForDrivers( "." );
	}

	unsigned int driverMode;
	const char *driverName = YnNode_GetStringByName( shellConfig, "shell.driver", "opengl" );
	if ( strcmp( driverName, "opengl" ) == 0 )
		driverMode = YN_CORE_GRAPHICS_OPENGL;
	else if ( strcmp( driverName, "vulkan" ) == 0 )
		driverMode = YN_CORE_GRAPHICS_VULKAN;
	else if ( strcmp( driverName, "software" ) == 0 )
		driverMode = YN_CORE_GRAPHICS_SOFTWARE;
	else
		driverMode = YN_CORE_GRAPHICS_OTHER;

	if ( ( windowViewport = YnCore_ShellInterface_CreateWindow( "Yin Game Engine", 1024, 768, false, driverMode ) ) == NULL )
	{
		YnCore_ShellInterface_DisplayMessageBox( YN_CORE_MESSAGE_ERROR, "Failed to create window!\n" );
		return EXIT_FAILURE;
	}

	if ( PlgSetDriver( driverName ) != PL_RESULT_SUCCESS )
	{
		if ( strcmp( driverName, "software" ) != 0 )
		{
			PrintWarn( "Driver init failed for \"%s\": %s\n", driverName, PlGetError() );
			Print( "Attempting to use software fallback...\n" );
			if ( PlgSetDriver( "software" ) != PL_RESULT_SUCCESS )
			{
				PrintWarn( "Failed to set fallback driver: %s\n", PlGetError() );
				return false;
			}
		}
		else
		{
			PrintWarn( "Driver init failed for \"%s\" and fallback failed: %s\n", driverName, PlGetError() );
			return false;
		}
	}

	return true;
}

int Launcher_Initialize( int argc, char **argv )
{
#if defined( _WIN32 ) && !defined( NDEBUG )
	/* stop buffering stdout! */
	setvbuf( stdout, NULL, _IONBF, 0 );

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	/* initialize the platform library */
	if ( PlInitialize( argc, argv ) != PL_RESULT_SUCCESS )
	{
		printf( "Failed to initialize Hei: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	if ( PlInitializeSubSystems( PL_SUBSYSTEM_IO ) != PL_RESULT_SUCCESS )
	{
		printf( "Failed to initialize IO subsystem: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	if ( PlHasCommandLineArgument( "-log" ) )
	{
		const char *path = PlGetCommandLineArgumentValue( "-log" );
		if ( path == NULL )
		{
			path = "log.txt";
		}
		PlSetupLogOutput( path );
	}

	launcherLog = PlAddLogLevel( "launcher", PL_COLOUR_WHITE, true );
	Print( "Log output initialized!\n" );

	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
	{
		PrintError( "Failed to initialize SDL2!\nSDL: %s\n", SDL_GetError() );
	}

	Common_Initialize();

	shellConfig = Common_GetConfig( "shell" );

	if ( !InitializeDisplay() )
	{
		PrintError( "Failed to initialize display!\nCheck debug logs.\n" );
	}

	if ( !YnCore_Initialize( NULL ) )
	{
		PrintError( "Failed to initialize engine!\nCheck debug logs.\n" );
	}

	// setup our timers, in this case we're just setting up our tick
	sdlTimer = SDL_AddTimer( YN_CORE_TICK_RATE, OS_TimerCallback, NULL );

	SDL_StartTextInput();

	while ( YnCore_IsEngineRunning() )
	{
		SDL_Event event;
		while ( SDL_PollEvent( &event ) )
		{
			switch ( event.type )
			{
				case SDL_USEREVENT:
					YnCore_TickFrame();
					break;

				case SDL_TEXTINPUT:
					YnCore_HandleTextEvent( event.text.text );
					break;

				case SDL_MOUSEWHEEL:
				{
					float x = ( event.wheel.x > 0 ) ? 1.0f : ( event.wheel.x < 0 ) ? -1.0f
					                                                               : 0.0f;
					float y = ( event.wheel.y > 0 ) ? 1.0f : ( event.wheel.y < 0 ) ? -1.0f
					                                                               : 0.0f;
					YnCore_HandleMouseWheelEvent( x, y );
					break;
				}
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					YnCore_HandleMouseButtonEvent( event.button.button, ( event.button.type == SDL_MOUSEBUTTONDOWN ) );
					break;
				case SDL_MOUSEMOTION:
					YnCore_HandleMouseMotionEvent( event.motion.x, event.motion.y );
					break;

				case SDL_KEYDOWN:
				case SDL_KEYUP:
				{
					int key = Sys_TranslateSDLKeyInput( event.key.keysym.sym );
					if ( key == KEY_INVALID )
					{
						//PrintWarn( "Unhandled key, %d\n", key );
						break;
					}

					keyStates[ key ] = ( event.type == SDL_KEYDOWN ) ? YN_CORE_INPUT_STATE_DOWN : YN_CORE_INPUT_STATE_NONE;

					YnCore_HandleKeyboardEvent( key, keyStates[ key ] );
					break;
				}

				case SDL_WINDOWEVENT:
				{
					if ( sdlWindow == NULL || event.window.windowID != SDL_GetWindowID( sdlWindow ) )
						break;

					switch ( event.window.event )
					{
						case SDL_WINDOWEVENT_RESIZED:
						case SDL_WINDOWEVENT_SIZE_CHANGED:
							//SDL_GL_GetDrawableSize( sdlWindow, &drawW, &drawW );
							// originally used the above but it kept returning bogus coords...
							SDL_GetWindowSize( sdlWindow, &drawW, &drawH );
							YnCore_Viewport_SetSize( windowViewport, drawW, drawH );
							break;
					}
					break;
				}
			}
		}

		YnCore_RenderFrame( windowViewport );

		SDL_GL_SwapWindow( sdlWindow );
	}

	SDL_StopTextInput();

	YnCore_Shutdown();

	return EXIT_SUCCESS;
}

int main( int argc, char **argv )
{
	return Launcher_Initialize( argc, argv );
}
