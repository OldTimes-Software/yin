/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include "net/net.h"
#include "protocol.h"

PL_EXTERN_C

typedef struct ServerClient ServerClient;

bool Server_Start( const char *ip, unsigned short port );

void Server_Initialize( void );
void Server_Shutdown( void );
void Server_DropClient( ServerClient *serverClient );
void Server_Tick( void );

unsigned short Server_GetPort( void );

PL_EXTERN_C_END
