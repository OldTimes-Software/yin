// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#include "fw_weapon.h"

typedef enum FWWeaponType
{
	FW_WEAPON_TYPE_PROJECTILE,
	FW_WEAPON_TYPE_TRACE,
} FWWeaponType;

typedef struct FWWeapon
{
	char *name;
	char *projectileName;
} FWWeapon;

void FW_Weapon_LoadTypes( void )
{
	NLNode *node = NL_LoadFile( "config/weapons.cfg.n", "weapons" );
	if ( node == NULL )
	{
		Game_Warning( "Failed to open weapons config: %s\n", NL_GetErrorMessage() );
		return;
	}

	NLNode *weaponObject = NL_GetFirstChild( node );
	while ( weaponObject != NULL )
	{
		const char *c = NL_GetStrByName( weaponObject, "name", NULL );

		weaponObject = NL_GetNextChild( weaponObject );
	}

	NL_DestroyNode( node );
}
