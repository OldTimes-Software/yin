// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include "game_private.h"
#include "game_component_transform.h"
#include "engine/public/engine_public_world.h"

static void Spawn( EntityComponent *self )
{
	self->userData = PL_NEW( ECTransform );
}

static NLNode *Serialize( EntityComponent *self, NLNode *root )
{
	NL_PushBackF32Array( root, "translation", ( float * ) &ECTRANSFORM( self )->translation, 3 );
	NL_PushBackF32Array( root, "scale", ( float * ) &ECTRANSFORM( self )->scale, 3 );
	NL_PushBackF32Array( root, "angles", ( float * ) &ECTRANSFORM( self )->angles, 3 );
	NL_PushBackI32( root, "sectorNum", ECTRANSFORM( self )->sectorNum );
	return root;
}

static NLNode *Deserialize( EntityComponent *self, NLNode *root )
{
	NLNode *child;
	if ( ( child = NL_GetChildByName( root, "translation" ) ) != NULL )
	{
		NL_GetF32Array( child, ( float * ) &ECTRANSFORM( self )->translation, 3 );
	}
	if ( ( child = NL_GetChildByName( root, "scale" ) ) != NULL )
	{
		NL_GetF32Array( child, ( float * ) &ECTRANSFORM( self )->scale, 3 );
	}
	if ( ( child = NL_GetChildByName( root, "angles" ) ) != NULL )
	{
		NL_GetF32Array( child, ( float * ) &ECTRANSFORM( self )->angles, 3 );
	}
	ECTRANSFORM( self )->sectorNum = NL_GetI32ByName( root, "sectorNum", -1 );
	return root;
}

static void Tick( EntityComponent *self )
{
	// if we're in the world, ensure we're attached to a valid sector
	World *world = Game_GetCurrentWorld();
	if ( world != NULL && ECTRANSFORM( self )->sectorNum == -1 )
	{
		Game_Warning( "Entity outside of world, attempting to relocate!\n" );

		WorldSector *sector = World_GetSectorByGlobalOrigin( world, &ECTRANSFORM( self )->translation );
		if ( sector != NULL )
		{
			//TODO: what fucking index is it!?
		}
		else
		{
			Game_Warning( "Failed to fetch sector by origin - falling to first sector!\n" );
			/*sector = World_GetSectorByNum( world, 0 );
			if ( sector == NULL )
			{
				Game_Warning( "No first sector, panic!!\n" );
			}*/
		}
	}
}

const EntityComponentCallbackTable *EntityComponent_Transform_GetCallbackTable( void )
{
	static EntityComponentCallbackTable callbackTable;
	PL_ZERO_( callbackTable );

	callbackTable.spawnFunction       = Spawn;
	callbackTable.serializeFunction   = Serialize;
	callbackTable.deserializeFunction = Deserialize;
	callbackTable.tickFunction        = Tick;

	return &callbackTable;
}
