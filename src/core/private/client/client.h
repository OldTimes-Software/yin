// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

PL_EXTERN_C

void YnCore_InitializeClient( void );
void YnCore_ShutdownClient( void );
void YnCore_DrawClient( YNCoreViewport *viewport );
void YnCore_TickClient( void );

void YnCore_Client_InitiateConnection( const char *ip, unsigned short port );
void YnCore_Client_Disconnect( void );

#define CLIENT_PRINT( FORMAT, ... ) \
	Console_Print( YINENGINE_LOG_CLIENT_INFORMATION, FORMAT, ##__VA_ARGS__ )
#define CLIENT_PRINT_WARNING( FORMAT, ... ) \
	Console_Print( YINENGINE_LOG_CLIENT_WARNING, FORMAT, ##__VA_ARGS__ )
#define YINENGINE_CLIENT_PRINT_ERROR( FORMAT, ... ) \
	Console_Print( YINENGINE_LOG_CLIENT_ERROR, FORMAT, ##__VA_ARGS__ )

PL_EXTERN_C_END
