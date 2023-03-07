/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

/**
 * CMN_ 	; used for macros and functions
 * Common 	; used for structs
 */

typedef enum CommonDataType
{
	CMN_DATATYPE_BOOL,

	CMN_DATATYPE_INT8,
	CMN_DATATYPE_INT16,
	CMN_DATATYPE_INT32,

	CMN_DATATYPE_UINT8,
	CMN_DATATYPE_UINT16,
	CMN_DATATYPE_UINT32,

	CMN_DATATYPE_FLOAT32,
	CMN_DATATYPE_FLOAT64,

	CMN_DATATYPE_POINTER,

	CMN_MAX_DATATYPES
} CommonDataType;

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

#if defined( COMMON_DLL )
#	include "kernel/plcore/include/plcore/pl_console.h"

#	include <assert.h>

extern int logLevelPrint;
extern int logLevelWarn;
#	define Message( FORMAT, ... ) PlLogWFunction( logLevelPrint, FORMAT, ##__VA_ARGS__ )
#	define Warning( FORMAT, ... ) PlLogWFunction( logLevelWarn, FORMAT, ##__VA_ARGS__ )
#endif

PL_EXTERN_C

void           Common_Initialize( void );
const char    *Common_GetAppDataDirectory( void );
struct YNNodeBranch *Common_GetConfig( const char *name );// attempts to fetch the specified config, otherwise returns an empty config
bool           Common_WriteConfig( struct YNNodeBranch *root, const char *name );

void Common_Pkg_WriteHeader( FILE *pack, unsigned int numFiles );
void Common_Pkg_AddData( FILE *pack, const char *path, const void *buf, size_t size );

PL_EXTERN_C_END
