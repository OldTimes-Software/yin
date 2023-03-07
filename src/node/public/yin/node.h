// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_filesystem.h>
#include <plcore/pl_math.h>

#include "common.h"

PL_EXTERN_C

typedef struct YNNodeBranch YNNodeBranch;

#define YN_NODE_DEFAULT_EXTENSION ".n"

typedef enum YNNodeErrorCode
{
	YN_NODE_ERROR_SUCCESS,

	YN_NODE_ERROR_IO_READ,  /* read failure */
	YN_NODE_ERROR_IO_WRITE, /* write failure */

	NL_ERROR_MEM_ALLOC, /* alloc failure */

	YN_NODE_ERROR_INVALID_ARGUMENT,
	YN_NODE_ERROR_INVALID_TYPE,     /* invalid node parent/child type */
	YN_NODE_ERROR_INVALID_ELEMENTS, /* unexpected number of elements */
} YNNodeErrorCode;

typedef enum NLFileType
{
	YN_NODE_FILE_INVALID = -1,
	YN_NODE_FILE_BINARY,
	YN_NODE_FILE_UTF8,

	NL_MAX_FILE_TYPES
} NLFileType;

typedef enum YNNodePropertyType
{
	YN_NODE_PROP_UNDEFINED = -1,

	YN_NODE_PROP_OBJ,
	NL_PROP_LINK, /* todo */
	YN_NODE_PROP_ARRAY,

	YN_NODE_PROP_STR,
	YN_NODE_PROP_BOOL,

	YN_NODE_PROP_F32, // float
	YN_NODE_PROP_F64, // double
	YN_NODE_PROP_I8,  // int8
	YN_NODE_PROP_I16, // int16
	YN_NODE_PROP_I32, // int32
	YN_NODE_PROP_I64, // int64
	YN_NODE_PROP_UI8, // uint8
	YN_NODE_PROP_UI16,// uint16
	YN_NODE_PROP_UI32,// uint32
	YN_NODE_PROP_UI64,// uint64

	YN_NODE_MAX_PROPERTY_TYPES
} YNNodePropertyType;

typedef union YNPropertyData
{
	float f32;
	double f64;
	int8_t i8;
	int16_t i16;
	int32_t i32;
	int64_t i64;
	uint8_t ui8;
	uint16_t ui16;
	uint32_t ui32;
	uint64_t ui64;
} YNPropertyData;

void YnNode_SetupLogs( void );

const char *YnNode_GetErrorMessage( void );
YNNodeErrorCode YnNode_GetError( void );

unsigned int YnNode_GetNumOfChildren( const YNNodeBranch *parent ); /* only valid for object/array */
YNNodeBranch *YnNode_GetFirstChild( YNNodeBranch *parent );
YNNodeBranch *YnNode_GetNextChild( YNNodeBranch *node );
YNNodeBranch *YnNode_GetChildByName( YNNodeBranch *parent, const char *name ); /* only valid for object */
YNNodeBranch *YnNode_GetParent( YNNodeBranch *node );

const char *YnNode_GetName( const YNNodeBranch *node );
YNNodePropertyType YnNode_GetType( const YNNodeBranch *node );

YNNodeErrorCode YnNode_GetBool( const YNNodeBranch *node, bool *dest );
YNNodeErrorCode YnNode_GetStr( const YNNodeBranch *node, char *dest, size_t length );
YNNodeErrorCode YnNode_GetF32( const YNNodeBranch *node, float *dest );
YNNodeErrorCode YnNode_GetF64( const YNNodeBranch *node, double *dest );
YNNodeErrorCode YnNode_GetI8( const YNNodeBranch *node, int8_t *dest );
YNNodeErrorCode YnNode_GetI16( const YNNodeBranch *node, int16_t *dest );
YNNodeErrorCode YnNode_GetI32( const YNNodeBranch *node, int32_t *dest );
YNNodeErrorCode YnNode_GetI64( const YNNodeBranch *node, int64_t *dest );
YNNodeErrorCode YnNode_GetUI8( const YNNodeBranch *node, uint8_t *dest );
YNNodeErrorCode YnNode_GetUI16( const YNNodeBranch *node, uint16_t *dest );
YNNodeErrorCode YnNode_GetUI32( const YNNodeBranch *node, uint32_t *dest );
YNNodeErrorCode YnNode_GetUI64( const YNNodeBranch *node, uint64_t *dest );

YNNodeErrorCode YnNode_GetStrArray( YNNodeBranch *parent, const char **buf, unsigned int numElements );
YNNodeErrorCode YnNode_GetI8Array( YNNodeBranch *parent, int8_t *buf, unsigned int numElements );
YNNodeErrorCode YnNode_GetI16Array( YNNodeBranch *parent, int16_t *buf, unsigned int numElements );
YNNodeErrorCode YnNode_GetI32Array( YNNodeBranch *parent, int32_t *buf, unsigned int numElements );
YNNodeErrorCode YnNode_GetUI32Array( YNNodeBranch *parent, uint32_t *buf, unsigned int numElements );
YNNodeErrorCode YnNode_GetF32Array( YNNodeBranch *parent, float *buf, unsigned int numElements );

bool YnNode_GetBoolByName( YNNodeBranch *root, const char *name, bool fallback );
const char *YnNode_GetStringByName( YNNodeBranch *node, const char *name, const char *fallback );
int32_t YnNode_GetI32ByName( YNNodeBranch *node, const char *name, int32_t fallback );
float YnNode_GetF32ByName( YNNodeBranch *node, const char *name, float fallback );
double NL_GetF64ByName( YNNodeBranch *node, const char *name, double fallback );

YNNodeBranch *YnNode_PushBackBranch( YNNodeBranch *parent, YNNodeBranch *child );
YNNodeBranch *YnNode_PushBackObject( YNNodeBranch *node, const char *name );
YNNodeBranch *YnNode_PushBackString( YNNodeBranch *parent, const char *name, const char *var );
YNNodeBranch *YnNode_PushBackBool( YNNodeBranch *parent, const char *name, bool var );
YNNodeBranch *YnNode_PushBackI8( YNNodeBranch *parent, const char *name, int8_t var );
YNNodeBranch *YnNode_PushBackI16( YNNodeBranch *parent, const char *name, int16_t var );
YNNodeBranch *YnNode_PushBackI32( YNNodeBranch *parent, const char *name, int32_t var );
YNNodeBranch *YnNode_PushBackUI32( YNNodeBranch *parent, const char *name, uint32_t var );
YNNodeBranch *YnNode_PushBackF32( YNNodeBranch *parent, const char *name, float var );
YNNodeBranch *YnNode_PushBackF64( YNNodeBranch *parent, const char *name, double var );

YNNodeBranch *YnNode_PushBackObjectArray( YNNodeBranch *parent, const char *name );
YNNodeBranch *YnNode_PushBackStringArray( YNNodeBranch *parent, const char *name, const char **array, unsigned int numElements );
YNNodeBranch *YnNode_PushBackI32Array( YNNodeBranch *parent, const char *name, const int32_t *array, unsigned int numElements );
YNNodeBranch *YnNode_PushBackF32Array( YNNodeBranch *parent, const char *name, const float *array, unsigned int numElements );

YNNodeBranch *YnNode_CopyBranch( YNNodeBranch *node );
void YnNode_DestroyBranch( YNNodeBranch *node );

YNNodeBranch *YnNode_ParseFile( PLFile *file, const char *objectType );
YNNodeBranch *YnNode_LoadFile( const char *path, const char *objectType );
bool YnNode_WriteFile( const char *path, YNNodeBranch *root, NLFileType fileType );

YNNodeBranch *YnNode_ParseBuffer( const char *buf, size_t length );

/* debugging */
void YnNode_PrintTree( YNNodeBranch *node, int index );

/* deserialisation/serialisation */

PLMatrix4 *NL_DS_DeserializeMatrix4( YNNodeBranch *in, PLMatrix4 *out );
float *NL_DS_DeserializeVector( YNNodeBranch *in, float *out, uint8_t numElements );

PLVector2 *NL_DS_DeserializeVector2( YNNodeBranch *in, PLVector2 *out );
YNNodeBranch *NL_DS_SerializeVector2( YNNodeBranch *parent, const char *name, const PLVector2 *vector2 );

PLVector3 *YnNode_DS_DeserializeVector3( YNNodeBranch *in, PLVector3 *out );
YNNodeBranch *NL_DS_SerializeVector3( YNNodeBranch *parent, const char *name, const PLVector3 *vector3 );

PLVector4 *NL_DS_DeserializeVector4( YNNodeBranch *in, PLVector4 *out );
PLQuaternion *NL_DS_DeserializeQuaternion( YNNodeBranch *in, PLQuaternion *out );

PLColour *NL_DS_DeserializeColour( YNNodeBranch *in, PLColour *out );
YNNodeBranch *NL_DS_SerializeColour( YNNodeBranch *parent, const char *name, const PLColour *colour );

PLColourF32 *YnNode_DS_DeserializeColourF32( YNNodeBranch *in, PLColourF32 *out );
YNNodeBranch *NL_DS_SerializeColourF32( YNNodeBranch *parent, const char *name, const PLColourF32 *colour );

struct PLGVertex *NL_DS_DeserializeVertex( YNNodeBranch *in, struct PLGVertex *out );
YNNodeBranch *NL_DS_SerializeVertex( YNNodeBranch *parent, const char *name, const struct PLGVertex *vertex );

struct PLCollisionAABB *NL_DS_DeserializeCollisionAABB( YNNodeBranch *in, struct PLCollisionAABB *out );
YNNodeBranch *NL_DS_SerializeCollisionAABB( YNNodeBranch *parent, const char *name, const struct PLCollisionAABB *collisionAabb );

PL_EXTERN_C_END
