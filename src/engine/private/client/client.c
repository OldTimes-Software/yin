// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "../engine_private.h"
#include "../net/net.h"

#include "client.h"
#include "client_input.h"

#include "game_interface.h"
#include "client_gui.h"

#include "editor/editor.h"
#include "renderer/renderer.h"
#include "audio/audio.h"

typedef struct ClientState
{
	NetSocket *netSocket;
	bool       isConnected;

	bool isEditMode;

	char userName[ 32 ];
} ClientState;
static ClientState clientState;

void Client_Initialize( void )
{
	CLIENT_PRINT( "Initializing client\n" );

	PL_ZERO_( clientState );

	YR_Initialize();
	Audio_Initialize();
	Editor_Initialize();

	Client_GUI_Initialize();

	Client_Input_Initialize();
}

void Client_Shutdown( void )
{
	Client_GUI_Shutdown();

	Editor_Shutdown();
	Audio_Shutdown();
	YR_Shutdown();
}

void Client_Draw( YRViewport *viewport )
{
	YR_BeginDraw( viewport );

	YR_DrawPerspective( viewport->camera, viewport );

	YR_DrawMenu( viewport );

	YR_EndDraw( viewport );
}

static void Client_HandleConnectionState( void )
{
	/* check if the client is connected to anything */
	if ( !clientState.isConnected )
	{
		/* socket hasn't been created, so... */
		if ( clientState.netSocket == NULL )
			return;

		NetConnectionState state = Net_GetConnectionStatus( clientState.netSocket );
		if ( state != NET_CONNECTION_CONNECTED )
		{
			if ( state == NET_CONNECTION_FAILED )
			{
				Client_Disconnect();
				CLIENT_PRINT_WARNING( "Connection failed!\n" );
			}
			return;
		}

		clientState.isConnected = true;
		CLIENT_PRINT( "Connected successfully!\n" );
	}
}

void Client_Tick( void )
{
	Client_Input_BeginFrame();

	Client_Input_Tick();

	/* edit mode is it's own special thing */
	if ( clientState.isEditMode )
	{
		Editor_Tick();
		return;
	}

	Client_GUI_Tick();

	Client_HandleConnectionState();

	Client_Input_EndFrame();

	Audio_Tick();
}

/**
 * Begin connection process - client will continue connecting per
 * tick until success or failure, and then begin handshake process.
 */
void Client_InitiateConnection( const char *ip, unsigned short port )
{
	clientState.netSocket = Net_OpenSocket( ip, port, false );
	if ( clientState.netSocket == NULL )
	{
		CLIENT_PRINT_WARNING( "Failed to open client socket!\n" );
		return;
	}

	CLIENT_PRINT( "Initiated connection to %s, pending...\n", ip );
}

void Client_Disconnect( void )
{
	if ( clientState.netSocket != NULL )
	{
		/* todo: let the server know first? */
		Net_CloseSocket( clientState.netSocket );
		clientState.netSocket = NULL;
	}
}
