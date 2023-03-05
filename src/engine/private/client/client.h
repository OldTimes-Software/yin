/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

PL_EXTERN_C

void Client_Initialize( void );
void Client_Shutdown( void );
void Client_Draw( YRViewport *viewport );
void Client_Tick( void );

void Client_InitiateConnection( const char *ip, unsigned short port );
void Client_Disconnect( void );

#define CLIENT_PRINT( FORMAT, ... ) \
	Console_Print( YINENGINE_LOG_CLIENT_INFORMATION, FORMAT, ##__VA_ARGS__ )
#define CLIENT_PRINT_WARNING( FORMAT, ... ) \
	Console_Print( YINENGINE_LOG_CLIENT_WARNING, FORMAT, ##__VA_ARGS__ )
#define YINENGINE_CLIENT_PRINT_ERROR( FORMAT, ... ) \
	Console_Print( YINENGINE_LOG_CLIENT_ERROR, FORMAT, ##__VA_ARGS__ )

PL_EXTERN_C_END
