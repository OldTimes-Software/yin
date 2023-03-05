/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl_filesystem.h>

#include "common.h"

PL_EXTERN_C

typedef struct NLNode NLNode;

#define NL_DEFAULT_EXTENSION ".n"

typedef enum NLErrorCode
{
	NL_ERROR_SUCCESS,

	NL_ERROR_IO_READ,  /* read failure */
	NL_ERROR_IO_WRITE, /* write failure */

	NL_ERROR_MEM_ALLOC, /* alloc failure */

	NL_ERROR_INVALID_ARGUMENT,
	NL_ERROR_INVALID_TYPE,     /* invalid node parent/child type */
	NL_ERROR_INVALID_ELEMENTS, /* unexpected number of elements */
} NLErrorCode;

typedef enum NLFileType
{
	NL_FILE_INVALID = -1,
	NL_FILE_BINARY,
	NL_FILE_UTF8,

	NL_MAX_FILE_TYPES
} NLFileType;

typedef enum NLPropertyType
{
	NL_PROP_UNDEFINED = -1,

	NL_PROP_OBJ,
	NL_PROP_LINK, /* todo */
	NL_PROP_ARRAY,

	NL_PROP_STR,
	NL_PROP_BOOL,

	NL_PROP_F32, // float
	NL_PROP_F64, // double
	NL_PROP_I8,  // int8
	NL_PROP_I16, // int16
	NL_PROP_I32, // int32
	NL_PROP_I64, // int64
	NL_PROP_UI8, // uint8
	NL_PROP_UI16,// uint16
	NL_PROP_UI32,// uint32
	NL_PROP_UI64,// uint64

	NL_MAX_PROPERTYTYPES
} NLPropertyType;

typedef union NLPropertyData_U
{
	float    f32;
	double   f64;
	int8_t   i8;
	int16_t  i16;
	int32_t  i32;
	int64_t  i64;
	uint8_t  ui8;
	uint16_t ui16;
	uint32_t ui32;
	uint64_t ui64;
} NLPropertyData_U;

void NL_SetupLogs( void );

const char *NL_GetErrorMessage( void );
NLErrorCode NL_GetError( void );

unsigned int NL_GetNumOfChildren( const NLNode *parent ); /* only valid for object/array */
NLNode      *NL_GetFirstChild( NLNode *parent );
NLNode      *NL_GetNextChild( NLNode *node );
NLNode      *NL_GetChildByName( NLNode *parent, const char *name ); /* only valid for object */
NLNode      *NL_GetChildByIndex( NLNode *parent, unsigned int i );  /* only valid for array */
NLNode      *NL_GetParent( NLNode *node );

const char    *NL_GetName( const NLNode *node );
NLPropertyType NL_GetType( const NLNode *node );

NLErrorCode NL_GetBool( const NLNode *node, bool *dest );
NLErrorCode NL_GetStr( const NLNode *node, char *dest, size_t length );
NLErrorCode NL_GetF32( const NLNode *node, float *dest );
NLErrorCode NL_GetF64( const NLNode *node, double *dest );
NLErrorCode NL_GetI8( const NLNode *node, int8_t *dest );
NLErrorCode NL_GetI16( const NLNode *node, int16_t *dest );
NLErrorCode NL_GetI32( const NLNode *node, int32_t *dest );
NLErrorCode NL_GetI64( const NLNode *node, int64_t *dest );
NLErrorCode NL_GetUI8( const NLNode *node, uint8_t *dest );
NLErrorCode NL_GetUI16( const NLNode *node, uint16_t *dest );
NLErrorCode NL_GetUI32( const NLNode *node, uint32_t *dest );
NLErrorCode NL_GetUI64( const NLNode *node, uint64_t *dest );

NLErrorCode NL_GetStrArray( NLNode *parent, const char **buf, unsigned int numElements );
NLErrorCode NL_GetI8Array( NLNode *parent, int8_t *buf, unsigned int numElements );
NLErrorCode NL_GetI16Array( NLNode *parent, int16_t *buf, unsigned int numElements );
NLErrorCode NL_GetI32Array( NLNode *parent, int32_t *buf, unsigned int numElements );
NLErrorCode NL_GetUI32Array( NLNode *parent, uint32_t *buf, unsigned int numElements );
NLErrorCode NL_GetF32Array( NLNode *parent, float *buf, unsigned int numElements );

bool        NL_GetBoolByName( NLNode *root, const char *name, bool fallback );
const char *NL_GetStrByName( NLNode *node, const char *name, const char *fallback );
int32_t     NL_GetI32ByName( NLNode *node, const char *name, int32_t fallback );
float       NL_GetF32ByName( NLNode *node, const char *name, float fallback );
double      NL_GetF64ByName( NLNode *node, const char *name, double fallback );

NLNode *NL_PushBackNode( NLNode *parent, NLNode *child );
NLNode *NL_PushBackObj( NLNode *node, const char *name );
NLNode *NL_PushBackStr( NLNode *parent, const char *name, const char *var );
NLNode *NL_PushBackBool( NLNode *parent, const char *name, bool var );
NLNode *NL_PushBackI8( NLNode *parent, const char *name, int8_t var );
NLNode *NL_PushBackI16( NLNode *parent, const char *name, int16_t var );
NLNode *NL_PushBackI32( NLNode *parent, const char *name, int32_t var );
NLNode *NL_PushBackUI32( NLNode *parent, const char *name, uint32_t var );
NLNode *NL_PushBackF32( NLNode *parent, const char *name, float var );
NLNode *NL_PushBackF64( NLNode *parent, const char *name, double var );

NLNode *NL_PushBackObjArray( NLNode *parent, const char *name );
NLNode *NL_PushBackStrArray( NLNode *parent, const char *name, const char **array, unsigned int numElements );
NLNode *NL_PushBackI32Array( NLNode *parent, const char *name, const int32_t *array, unsigned int numElements );
NLNode *NL_PushBackF32Array( NLNode *parent, const char *name, const float *array, unsigned int numElements );

NLNode *NL_CopyNode( NLNode *node );
void    NL_DestroyNode( NLNode *node );

NLNode *NL_ParseFile( PLFile *file, const char *objectType );
NLNode *NL_LoadFile( const char *path, const char *objectType );
bool NL_WriteFile( const char *path, NLNode *root, NLFileType fileType );

NLNode *NL_ParseBuffer( const char *buf, size_t length );

/* debugging */
void NL_PrintNodeTree( NLNode *node, int index );

/* deserialisation/serialisation */

PLMatrix4 *NL_DS_DeserializeMatrix4( NLNode *in, PLMatrix4 *out );
float     *NL_DS_DeserializeVector( NLNode *in, float *out, uint8_t numElements );

PLVector2 *NL_DS_DeserializeVector2( NLNode *in, PLVector2 *out );
NLNode    *NL_DS_SerializeVector2( NLNode *parent, const char *name, const PLVector2 *vector2 );

PLVector3 *NL_DS_DeserializeVector3( NLNode *in, PLVector3 *out );
NLNode    *NL_DS_SerializeVector3( NLNode *parent, const char *name, const PLVector3 *vector3 );

PLVector4    *NL_DS_DeserializeVector4( NLNode *in, PLVector4 *out );
PLQuaternion *NL_DS_DeserializeQuaternion( NLNode *in, PLQuaternion *out );

PLColour *NL_DS_DeserializeColour( NLNode *in, PLColour *out );
NLNode   *NL_DS_SerializeColour( NLNode *parent, const char *name, const PLColour *colour );

PLColourF32 *NL_DS_DeserializeColourF32( NLNode *in, PLColourF32 *out );
NLNode      *NL_DS_SerializeColourF32( NLNode *parent, const char *name, const PLColourF32 *colour );

struct PLGVertex *NL_DS_DeserializeVertex( NLNode *in, struct PLGVertex *out );
NLNode           *NL_DS_SerializeVertex( NLNode *parent, const char *name, const struct PLGVertex *vertex );

struct PLCollisionAABB *NL_DS_DeserializeCollisionAABB( NLNode *in, struct PLCollisionAABB *out );
NLNode                 *NL_DS_SerializeCollisionAABB( NLNode *parent, const char *name, const struct PLCollisionAABB *collisionAabb );

PL_EXTERN_C_END
