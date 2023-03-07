// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <assert.h>

#include <plcore/pl_physics.h>
#include <plgraphics/plg_mesh.h>

#include "node_private.h"

PLMatrix4 *NL_DS_DeserializeMatrix4( YNNodeBranch *in, PLMatrix4 *out )
{
	if ( in == NULL )
	{
		return NULL;
	}

	PlClearMatrix4( out );

	if ( in->childType != YN_NODE_PROP_F32 )
		return NULL;

	YNNodeBranch *c = YnNode_GetFirstChild( in );
	for ( uint8_t i = 0; i < 16; ++i )
	{
		if ( c == NULL || ( YnNode_GetF32( c, &out->m[ i ] ) != YN_NODE_ERROR_SUCCESS ) )
			break;

		c = YnNode_GetNextChild( c );
	}

	return out;
}

float *NL_DS_DeserializeVector( YNNodeBranch *in, float *out, uint8_t numElements )
{
	if ( in == NULL )
		return NULL;

	assert( numElements != 0 && numElements < 4 );
	const char *elements[] = { "x", "y", "z", "w" };
	for ( uint8_t i = 0; i < numElements; ++i )
		out[ i ] = YnNode_GetF32ByName( in, elements[ i ], 0.0f );

	return out;
}

PLVector2    *NL_DS_DeserializeVector2( YNNodeBranch *in, PLVector2 *out ) { return ( PLVector2    *) NL_DS_DeserializeVector( in, ( float    *) out, 2 ); }
PLVector3    *YnNode_DS_DeserializeVector3( YNNodeBranch *in, PLVector3 *out ) { return ( PLVector3    *) NL_DS_DeserializeVector( in, ( float    *) out, 3 ); }
PLVector4    *NL_DS_DeserializeVector4( YNNodeBranch *in, PLVector4 *out ) { return ( PLVector4    *) NL_DS_DeserializeVector( in, ( float    *) out, 4 ); }
PLQuaternion *NL_DS_DeserializeQuaternion( YNNodeBranch *in, PLQuaternion *out ) { return ( PLQuaternion * ) NL_DS_DeserializeVector( in, ( float * ) out, 4 ); }

YNNodeBranch *NL_DS_SerializeColour( YNNodeBranch *parent, const char *name, const PLColour *colour )
{
	YNNodeBranch *object = YnNode_PushBackObject( parent, name );
	YnNode_PushBackI8( object, "r", colour->r );
	YnNode_PushBackI8( object, "g", colour->g );
	YnNode_PushBackI8( object, "b", colour->b );
	YnNode_PushBackI8( object, "a", colour->a );
	return object;
}

PLColour *NL_DS_DeserializeColour( YNNodeBranch *in, PLColour *out )
{
	if ( in == NULL )
		return NULL;

	out->r = YnNode_GetI32ByName( in, "r", 255 );
	out->g = YnNode_GetI32ByName( in, "g", 255 );
	out->b = YnNode_GetI32ByName( in, "b", 255 );
	out->a = YnNode_GetI32ByName( in, "a", 255 );
	return out;
}

YNNodeBranch *NL_DS_SerializeColourF32( YNNodeBranch *parent, const char *name, const PLColourF32 *colour )
{
	YNNodeBranch *object = YnNode_PushBackObject( parent, name );
	YnNode_PushBackF32( object, "r", colour->r );
	YnNode_PushBackF32( object, "g", colour->g );
	YnNode_PushBackF32( object, "b", colour->b );
	YnNode_PushBackF32( object, "a", colour->a );
	return object;
}

PLColourF32 *YnNode_DS_DeserializeColourF32( YNNodeBranch *in, PLColourF32 *out )
{
	if ( in == NULL )
		return NULL;

	out->r = YnNode_GetF32ByName( in, "r", 1.0f );
	out->g = YnNode_GetF32ByName( in, "g", 1.0f );
	out->b = YnNode_GetF32ByName( in, "b", 1.0f );
	out->a = YnNode_GetF32ByName( in, "a", 1.0f );
	return out;
}

/****************************************
 ****************************************/

YNNodeBranch *NL_DS_SerializeVertex( YNNodeBranch *parent, const char *name, const PLGVertex *vertex )
{
	YNNodeBranch *object = YnNode_PushBackObject( parent, name );
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

PLGVertex *NL_DS_DeserializeVertex( YNNodeBranch *in, PLGVertex *out )
{
	if ( in == NULL )
		return NULL;

	YnNode_DS_DeserializeVector3( YnNode_GetChildByName( in, "position" ), &out->position );
	NL_DS_DeserializeColour( YnNode_GetChildByName( in, "colour" ), &out->colour );
	YnNode_DS_DeserializeVector3( YnNode_GetChildByName( in, "normal" ), &out->normal );
	YnNode_DS_DeserializeVector3( YnNode_GetChildByName( in, "tangent" ), &out->tangent );
	YnNode_DS_DeserializeVector3( YnNode_GetChildByName( in, "bitangent" ), &out->bitangent );
	NL_DS_DeserializeVector2( YnNode_GetChildByName( in, "uv" ), &out->st[ 0 ] );
	return out;
}

/****************************************
 ****************************************/

PLCollisionAABB *NL_DS_DeserializeCollisionAABB( YNNodeBranch *in, PLCollisionAABB *out )
{
	if ( in == NULL )
		return NULL;

	YnNode_DS_DeserializeVector3( YnNode_GetChildByName( in, "mins" ), &out->mins );
	YnNode_DS_DeserializeVector3( YnNode_GetChildByName( in, "maxs" ), &out->maxs );
	YnNode_DS_DeserializeVector3( YnNode_GetChildByName( in, "origin" ), &out->origin );
	YnNode_DS_DeserializeVector3( YnNode_GetChildByName( in, "absOrigin" ), &out->absOrigin );
	return out;
}

/****************************************
 * SERIALISATION
 ****************************************/

YNNodeBranch *NL_DS_SerializeVector2( YNNodeBranch *parent, const char *name, const PLVector2 *vector2 )
{
	YNNodeBranch *object = YnNode_PushBackObject( parent, name );
	YnNode_PushBackF32( object, "x", vector2->x );
	YnNode_PushBackF32( object, "y", vector2->y );
	return object;
}

YNNodeBranch *NL_DS_SerializeVector3( YNNodeBranch *parent, const char *name, const PLVector3 *vector3 )
{
	YNNodeBranch *object = YnNode_PushBackObject( parent, name );
	YnNode_PushBackF32( object, "x", vector3->x );
	YnNode_PushBackF32( object, "y", vector3->y );
	YnNode_PushBackF32( object, "z", vector3->z );
	return object;
}

YNNodeBranch *NL_DS_SerializeCollisionAABB( YNNodeBranch *parent, const char *name, const PLCollisionAABB *collisionAabb )
{
	YNNodeBranch *object = YnNode_PushBackObject( parent, name );
	NL_DS_SerializeVector3( object, "mins", &collisionAabb->mins );
	NL_DS_SerializeVector3( object, "maxs", &collisionAabb->maxs );
	NL_DS_SerializeVector3( object, "origin", &collisionAabb->origin );
	NL_DS_SerializeVector3( object, "absOrigin", &collisionAabb->absOrigin );
	return object;
}
