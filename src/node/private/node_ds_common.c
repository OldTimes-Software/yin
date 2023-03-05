/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <assert.h>

#include <plcore/pl_physics.h>
#include <plgraphics/plg_mesh.h>

#include "node_private.h"

PLMatrix4 *NL_DS_DeserializeMatrix4( NLNode *in, PLMatrix4 *out )
{
	if ( in == NULL )
	{
		return NULL;
	}

	PlClearMatrix4( out );

	if ( in->childType != NL_PROP_F32 )
		return NULL;

	NLNode *c = NL_GetFirstChild( in );
	for ( uint8_t i = 0; i < 16; ++i )
	{
		if ( c == NULL || ( NL_GetF32( c, &out->m[ i ] ) != NL_ERROR_SUCCESS ) )
			break;

		c = NL_GetNextChild( c );
	}

	return out;
}

float *NL_DS_DeserializeVector( NLNode *in, float *out, uint8_t numElements )
{
	if ( in == NULL )
		return NULL;

	assert( numElements != 0 && numElements < 4 );
	const char *elements[] = { "x", "y", "z", "w" };
	for ( uint8_t i = 0; i < numElements; ++i )
		out[ i ] = NL_GetF32ByName( in, elements[ i ], 0.0f );

	return out;
}

PLVector2    *NL_DS_DeserializeVector2( NLNode *in, PLVector2 *out ) { return ( PLVector2    *) NL_DS_DeserializeVector( in, ( float    *) out, 2 ); }
PLVector3    *NL_DS_DeserializeVector3( NLNode *in, PLVector3 *out ) { return ( PLVector3    *) NL_DS_DeserializeVector( in, ( float    *) out, 3 ); }
PLVector4    *NL_DS_DeserializeVector4( NLNode *in, PLVector4 *out ) { return ( PLVector4    *) NL_DS_DeserializeVector( in, ( float    *) out, 4 ); }
PLQuaternion *NL_DS_DeserializeQuaternion( NLNode *in, PLQuaternion *out ) { return ( PLQuaternion * ) NL_DS_DeserializeVector( in, ( float * ) out, 4 ); }

NLNode *NL_DS_SerializeColour( NLNode *parent, const char *name, const PLColour *colour )
{
	NLNode *object = NL_PushBackObj( parent, name );
	NL_PushBackI8( object, "r", colour->r );
	NL_PushBackI8( object, "g", colour->g );
	NL_PushBackI8( object, "b", colour->b );
	NL_PushBackI8( object, "a", colour->a );
	return object;
}

PLColour *NL_DS_DeserializeColour( NLNode *in, PLColour *out )
{
	if ( in == NULL )
		return NULL;

	out->r = NL_GetI32ByName( in, "r", 255 );
	out->g = NL_GetI32ByName( in, "g", 255 );
	out->b = NL_GetI32ByName( in, "b", 255 );
	out->a = NL_GetI32ByName( in, "a", 255 );
	return out;
}

NLNode *NL_DS_SerializeColourF32( NLNode *parent, const char *name, const PLColourF32 *colour )
{
	NLNode *object = NL_PushBackObj( parent, name );
	NL_PushBackF32( object, "r", colour->r );
	NL_PushBackF32( object, "g", colour->g );
	NL_PushBackF32( object, "b", colour->b );
	NL_PushBackF32( object, "a", colour->a );
	return object;
}

PLColourF32 *NL_DS_DeserializeColourF32( NLNode *in, PLColourF32 *out )
{
	if ( in == NULL )
		return NULL;

	out->r = NL_GetF32ByName( in, "r", 1.0f );
	out->g = NL_GetF32ByName( in, "g", 1.0f );
	out->b = NL_GetF32ByName( in, "b", 1.0f );
	out->a = NL_GetF32ByName( in, "a", 1.0f );
	return out;
}

/****************************************
 ****************************************/

NLNode *NL_DS_SerializeVertex( NLNode *parent, const char *name, const PLGVertex *vertex )
{
	NLNode *object = NL_PushBackObj( parent, name );
	NL_DS_SerializeVector3( object, "position", &vertex->position );
	NL_DS_SerializeColour( object, "colour", &vertex->colour );
	if ( !PlCompareVector3( &vertex->normal, &pl_vecOrigin3 ) )
		NL_DS_SerializeVector3( object, "normal", &vertex->normal );
	if ( !PlCompareVector3( &vertex->tangent, &pl_vecOrigin3 ) )
		NL_DS_SerializeVector3( object, "tangent", &vertex->tangent );
	if ( !PlCompareVector3( &vertex->bitangent, &pl_vecOrigin3 ) )
		NL_DS_SerializeVector3( object, "bitangent", &vertex->tangent );
	if ( !PlCompareVector2( &vertex->st[ 0 ], &pl_vecOrigin2 ) )
		NL_DS_SerializeVector2( object, "uv", &vertex->st[ 0 ] );
	return object;
}

PLGVertex *NL_DS_DeserializeVertex( NLNode *in, PLGVertex *out )
{
	if ( in == NULL )
		return NULL;

	NL_DS_DeserializeVector3( NL_GetChildByName( in, "position" ), &out->position );
	NL_DS_DeserializeColour( NL_GetChildByName( in, "colour" ), &out->colour );
	NL_DS_DeserializeVector3( NL_GetChildByName( in, "normal" ), &out->normal );
	NL_DS_DeserializeVector3( NL_GetChildByName( in, "tangent" ), &out->tangent );
	NL_DS_DeserializeVector3( NL_GetChildByName( in, "bitangent" ), &out->bitangent );
	NL_DS_DeserializeVector2( NL_GetChildByName( in, "uv" ), &out->st[ 0 ] );
	return out;
}

/****************************************
 ****************************************/

PLCollisionAABB *NL_DS_DeserializeCollisionAABB( NLNode *in, PLCollisionAABB *out )
{
	if ( in == NULL )
		return NULL;

	NL_DS_DeserializeVector3( NL_GetChildByName( in, "mins" ), &out->mins );
	NL_DS_DeserializeVector3( NL_GetChildByName( in, "maxs" ), &out->maxs );
	NL_DS_DeserializeVector3( NL_GetChildByName( in, "origin" ), &out->origin );
	NL_DS_DeserializeVector3( NL_GetChildByName( in, "absOrigin" ), &out->absOrigin );
	return out;
}

/****************************************
 * SERIALISATION
 ****************************************/

NLNode *NL_DS_SerializeVector2( NLNode *parent, const char *name, const PLVector2 *vector2 )
{
	NLNode *object = NL_PushBackObj( parent, name );
	NL_PushBackF32( object, "x", vector2->x );
	NL_PushBackF32( object, "y", vector2->y );
	return object;
}

NLNode *NL_DS_SerializeVector3( NLNode *parent, const char *name, const PLVector3 *vector3 )
{
	NLNode *object = NL_PushBackObj( parent, name );
	NL_PushBackF32( object, "x", vector3->x );
	NL_PushBackF32( object, "y", vector3->y );
	NL_PushBackF32( object, "z", vector3->z );
	return object;
}

NLNode *NL_DS_SerializeCollisionAABB( NLNode *parent, const char *name, const PLCollisionAABB *collisionAabb )
{
	NLNode *object = NL_PushBackObj( parent, name );
	NL_DS_SerializeVector3( object, "mins", &collisionAabb->mins );
	NL_DS_SerializeVector3( object, "maxs", &collisionAabb->maxs );
	NL_DS_SerializeVector3( object, "origin", &collisionAabb->origin );
	NL_DS_SerializeVector3( object, "absOrigin", &collisionAabb->absOrigin );
	return object;
}
